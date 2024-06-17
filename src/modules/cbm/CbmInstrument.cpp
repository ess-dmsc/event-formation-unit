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

#define IBM_ADC_MASK 0xFFFFFF

namespace cbm {

using namespace esstime;

/// \brief load configuration and calibration files
CbmInstrument::CbmInstrument(
    struct Counters &Counters, Config &Config,
    const HashMap2D<EV44Serializer> &Ev44serializerMap,
    const HashMap2D<fbserializer::HistogramSerializer<int32_t>>
        &HistogramSerializerMap)

    : counters(Counters), Conf(Config), Ev44SerializerMap(Ev44serializerMap),
      HistogramSerializerMap(HistogramSerializerMap) {

  ESSHeaderParser.setMaxPulseTimeDiff(Conf.Parms.MaxPulseTimeDiffNS);
  ESSHeaderParser.Packet.Time.setMaxTOF(Conf.Parms.MaxTOFNS);
}

void CbmInstrument::processMonitorReadouts(void) {
  ESSReadout::ESSReferenceTime &RefTime = ESSHeaderParser.Packet.Time;
  // All readouts are now potentially valid, negative TOF is not
  // possible, or 0 ADC values, but rings and fens could still be outside the
  // configured range, also illegal time intervals can be detected here

  for (auto &Serializer : Ev44SerializerMap.toValuesList()) {
    Serializer->checkAndSetReferenceTime(
        ESSHeaderParser.Packet.Time.getRefTimeUInt64());
    /// \todo sometimes PrevPulseTime maybe?
  }

  for (auto &Serializer : HistogramSerializerMap.toValuesList()) {
    Serializer->checkAndSetReferenceTime(
        ESSHeaderParser.Packet.Time.getRefTimeNS());
    /// \todo sometimes PrevPulseTime maybe?
  }

  XTRACE(DATA, DEB, "processMonitorReadouts() - has %zu entries",
         CbmReadoutParser.Result.size());

  for (const auto &Readout : CbmReadoutParser.Result) {

    XTRACE(DATA, DEB,
           "readout: FiberId %d, FENId %d, POS %d, Type %d, Channel %d, ADC "
           "%d, TimeLow %d",
           Readout.FiberId, Readout.FENId, Readout.Pos, Readout.Type,
           Readout.Channel, Readout.ADC, Readout.TimeLow);

    int Ring = Readout.FiberId / 2;
    if (Ring != Conf.Parms.MonitorRing) {
      XTRACE(DATA, WAR, "Invalid ring %u (expect %u) for monitor readout", Ring,
             Conf.Parms.MonitorRing);
      counters.RingCfgError++;
      continue;
    }

    ESSTime ReadoutTime = ESSTime(Readout.TimeHigh, Readout.TimeLow);

    /// \todo: This function can come back with a valid TOF based on the
    /// previous pulse time configured on the RefTime class. This means the
    /// result looks valid, but for many detectors this TOF will be added to
    /// the Serializer with the current PulseTime. Handling of this case
    /// should be reviewed, maybe this will lead to incorrect statistics.
    /// Currently we just count the event for statistics.
    uint64_t TimeOfFlight = RefTime.getTOF(ReadoutTime);

    if (TimeOfFlight == RefTime.InvalidTOF) {
      XTRACE(DATA, WAR,
             "No valid TOF from pulse time: %" PRIu32 " and %" PRIu64
             "readout time",
             RefTime.getPrevRefTimeUInt64(), ReadoutTime.toNS().count());
      counters.TimeError++;
      continue;
    }

    // Check for out_of_range errors thrown by the HashMap2D, which contains the
    // serializers
    try {

      if (Readout.Type == CbmType::IBM) {
        auto AdcValue = Readout.NPos & IBM_ADC_MASK; // Extract lower 24 bits

        HistogramSerializerMap.get(Readout.FENId, Readout.Channel)
            ->addEvent(TimeOfFlight, AdcValue);

        counters.IBMReadoutsProcessed++;
      }

      else if (Readout.Type == CbmType::TTL) {

        // Register pixels according to the topology map offset and range
        auto &PixelOffset =
            Conf.TopologyMapPtr->get(Readout.FENId, Readout.Channel)
                ->pixelOffset;
        auto &PixelRange =
            Conf.TopologyMapPtr->get(Readout.FENId, Readout.Channel)
                ->pixelRange;

        for (int i = 0; i < PixelRange; i++) {
          int PixelId = PixelOffset + i;
          XTRACE(DATA, DEB,
                 "CbmType: %" PRIu8 " Pixel: %" PRIu32 " TOF %" PRIu64 "ns",
                 Readout.Type, PixelId, TimeOfFlight);

          Ev44SerializerMap.get(Readout.FENId, Readout.Channel)
              ->addEvent(TimeOfFlight, PixelId);
        }
        counters.TTLReadoutsProcessed++;
      } else {
        XTRACE(DATA, WAR, "Invalid CbmType %d", Readout.Type);
        counters.TypeNotSupported++;
        continue;
      }
    } catch (std::out_of_range &e) {
      LOG(UTILS, Sev::Warning,
          "No serializer configured for FEN %d, Channel %d", Readout.FENId,
          Readout.Channel);

      counters.NoSerializerCfgError++;
      continue;
    }

    counters.CbmCounts++;
  }

  // Update the time statistics
  counters.TimeStats = RefTime.Stats;
}

} // namespace cbm
