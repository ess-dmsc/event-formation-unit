// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Cbm is responsible for readout validation and event formation for
/// the common beam monitor (CBM) instrument
///
//===----------------------------------------------------------------------===//

#include <modules/cbm/CbmInstrument.h>
#include <stdexcept>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

/// \brief load configuration and calibration files
CbmInstrument::CbmInstrument(
    struct Counters &Counters, Config &Config,
    const HashMap2D<EV44Serializer> &Ev44serializerMap,
    const HashMap2D<fbserializer::HistogramSerializer<int32_t>>
        &HistogramSerializerMap)

    : counters(Counters), Conf(Config), Ev44SerializerMap(Ev44serializerMap),
      HistogramSerializerMap(HistogramSerializerMap) {

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.Parms.MaxPulseTimeDiffNS);
}

void CbmInstrument::processMonitorReadouts(void) {
  ESSReadout::ESSReferenceTime &TimeRef = ESSReadoutParser.Packet.Time;
  // All readouts are now potentially valid, negative TOF is not
  // possible, or 0 ADC values, but rings and fens could still be outside the
  // configured range, also illegal time intervals can be detected here

  for (auto &Serializer : Ev44SerializerMap.toValuesList()) {
    Serializer->checkAndSetReferenceTime(
        ESSReadoutParser.Packet.Time.getRefTimeUInt64());
    /// \todo sometimes PrevPulseTime maybe?
  }

  for (auto &Serializer : HistogramSerializerMap.toValuesList()) {
    Serializer->checkAndSetReferenceTime(
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

    try {
      if (readout.Type == CbmType::IBM) {
        auto AdcValue = readout.NPos & 0xFFFFFF; // Extract lower 24 bits

        HistogramSerializerMap.get(readout.FENId, readout.Channel)
            ->addEvent(TimeOfFlight, AdcValue);

        counters.IBMReadouts++;
      }

      else if (readout.Type == CbmType::TTL) {

        // Registering Pixels according to the topology map offset and range
        int PixelOffset =
            Conf.TopologyMapPtr->get(readout.FENId, readout.Channel)
                ->pixelOffset;
        int PixelRange =
            Conf.TopologyMapPtr->get(readout.FENId, readout.Channel)->pixelRang;

        for (int i = 0; i < PixelRange; i++) {
          int PixelId = PixelOffset + i;
          XTRACE(DATA, DEB,
                 "CbmType: %" PRIu8 " Pixel: %" PRIu32 " TOF %" PRIu64 "ns",
                 readout.Type, PixelId, TimeOfFlight);

          Ev44SerializerMap.get(readout.FENId, readout.Channel)
              ->addEvent(TimeOfFlight, PixelId);

          counters.TTLReadouts++;
        }
      } else {
        XTRACE(DATA, WAR, "Invalid CbmType %d", readout.Type);
        counters.TypeNotSupported++;
        continue;
      }
    } catch (std::out_of_range &e) {
      LOG(UTILS, Sev::Warning,
          "No serializer configured for FEN %d, Channel %d", readout.FENId,
          readout.Channel);

      counters.NoSerializerCfgError++;
      continue;
    }

    counters.CbmCounts++;
  }
}

} // namespace cbm
