// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM is a dedicated module for TTL triggered beam monitor
///
//===----------------------------------------------------------------------===//

#include <modules/cbm/CbmInstrument.h>
#include <stdexcept>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

/// \brief load configuration and calibration files
CbmInstrument::CbmInstrument(
    struct Counters &Counters, BaseSettings &Settings,
    HashMap2D<EV44Serializer> &Ev44serializerMap,
    HashMap2D<fbserializer::HistogramSerializer<int32_t>>
        &HistogramSerializerMap)

    : counters(Counters), Settings(Settings),
      Ev44SerializerMap(Ev44serializerMap),
      HistogramSerializerMap(HistogramSerializerMap) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  Conf = Config(Settings.ConfigFile);
  Conf.loadAndApply();

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.Parms.MaxPulseTimeDiffNS);
}

void CbmInstrument::processMonitorReadouts(void) {
  ESSReadout::ESSReferenceTime &TimeRef = ESSReadoutParser.Packet.Time;
  // All readouts are now potentially valid, negative TOF is not
  // possible, or 0 ADC values, but rings and fens could still be outside the
  // configured range, also illegal time intervals can be detected here

  for (auto &Serializer : Ev44SerializerMap.getAllValues()) {
    Serializer.second->checkAndSetReferenceTime(
        ESSReadoutParser.Packet.Time.getRefTimeUInt64());
    /// \todo sometimes PrevPulseTime maybe?
  }

  for (auto &Serializer : HistogramSerializerMap.getAllValues()) {
    Serializer.second->checkAndSetReferenceTime(
        ESSReadoutParser.Packet.Time.getRefTimeNS());
    /// \todo sometimes PrevPulseTime maybe?
  }

  XTRACE(DATA, DEB, "processMonitorReadouts() - has %zu entries",
         CbmParser.Result.size());

  for (const auto &readout : CbmParser.Result) {

    XTRACE(DATA, DEB,
           "readout: FiberId %d, FENId %d, POS %d, Type %d, Channel %d, ADC "
           "%d, TimeLow %d",
           readout.FiberId, readout.FENId, readout.Pos, readout.Type,
           readout.Channel, readout.ADC, readout.TimeLow);

    int Ring = readout.FiberId / 2;
    if (Ring != Conf.Parms.MonitorRing) {
      XTRACE(DATA, WAR, "Invalid ring %u (expect %u) for monitor readout", Ring,
             Conf.Parms.MonitorRing);
      counters.RingCfgErrors++;
      continue;
    }

    uint64_t TimeNS = ESSReadoutParser.Packet.Time.getRefTimeUInt64();
    XTRACE(DATA, DEB, "TimeRef PrevTime %" PRIi64 "",
           TimeRef.getPrevRefTimeUInt64());
    XTRACE(DATA, DEB, "TimeRef CurrTime %" PRIi64 "",
           TimeRef.getRefTimeUInt64());
    XTRACE(DATA, DEB, "Time of readout  %" PRIi64 "", TimeNS);

    uint64_t TimeOfFlight = 0;
    if (TimeRef.getRefTimeUInt64() > TimeNS) {
      TimeOfFlight = TimeNS - TimeRef.getPrevRefTimeUInt64();
    } else {
      TimeOfFlight = TimeNS - TimeRef.getRefTimeUInt64();
    }

    if (TimeOfFlight > Conf.Parms.MaxTOFNS) {
      XTRACE(DATA, WAR, "TOF larger than %u ns", Conf.Parms.MaxTOFNS);
      counters.TOFErrors++;
      continue;
    }

    CbmType type{1};
    try {
      type = CbmType(readout.Type);
    } catch (std::invalid_argument &e) {
      XTRACE(DATA, WAR, "Invalid data type %d (valid: %d - %d)", readout.Type,
             CbmType::MIN, CbmType::MAX);
      continue;
    }

    if (type == CbmType::IBM) {
      auto AdcValue = readout.NPos & 0xFFFFFF; // Extract lower 24 bits
      try {
        HistogramSerializerMap.get(readout.FENId, readout.Channel)
            ->addEvent(TimeOfFlight, AdcValue);
      } catch (std::out_of_range &e) {
        LOG(UTILS, Sev::Error, "No serializer configured for FEN %d, Channel %d",
            readout.FENId, readout.Channel);

        counters.NoSerializerCfgError++;
        continue;
      }

      counters.IBMReadouts++;
    }

    if (type == CbmType::TTL) {

      /// \todo calculate pixel id according to the config offsets
      uint32_t PixelId = 1;
      XTRACE(DATA, DEB, "CbmType: %s Pixel: %" PRIu32 " TOF %" PRIu64 "ns",
             type.to_string(), PixelId, TimeOfFlight);
      try {
        Ev44SerializerMap.get(readout.FENId, readout.Channel)
            ->addEvent(TimeOfFlight, PixelId);
      } catch (std::out_of_range &e) {
        LOG(UTILS, Sev::Error, "No serializer configured for FEN %d, Channel %d",
            readout.FENId, readout.Channel);

        counters.NoSerializerCfgError++;
        continue;
      }

      counters.TTLReadouts++;
    }

    counters.MonitorCounts++;
  }
}

} // namespace cbm
