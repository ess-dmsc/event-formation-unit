// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Cbm is responsible for readout validation and event formation for
/// the common beam monitor (CBM) instrument
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <modules/cbm/CbmInstrument.h>
#include <modules/cbm/CbmTypes.h>
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

void CbmInstrument::processMonitorReadouts() {
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
           "%d, TimeHigh %d, TimeLow %d",
           Readout.FiberId, Readout.FENId, Readout.Pos, Readout.Type,
           Readout.Channel, Readout.ADC, Readout.TimeHigh, Readout.TimeLow);

    int Ring = Readout.FiberId / 2;
    if (Ring != Conf.Parms.MonitorRing) {
      XTRACE(DATA, WAR, "Invalid ring %u (expect %u) for monitor readout", Ring,
             Conf.Parms.MonitorRing);
      counters.RingCfgError++;
      continue;
    }

    ESSTime ReadoutTime = ESSTime(Readout.TimeHigh, Readout.TimeLow);

    /// Calculates the time of flight (TOF) for the readout based on the
    /// reference time and the readout time. If the readout time is smaller than
    /// the reference time, it means that tof would be negative and this fucntion
    /// returns the TOF according to the previous reference time. This is includes
    /// delayed readouts into the current pulse statistics.
    /// \note: If time calculation fails, the function returns InvalidTOF
    uint64_t TimeOfFlight = RefTime.getTOF(ReadoutTime);

    if (TimeOfFlight == RefTime.InvalidTOF) {
      XTRACE(DATA, WAR,
             "No valid TOF from pulse time: %" PRIu32 " and %" PRIu64
             " readout time",
             RefTime.getPrevRefTimeUInt64(), ReadoutTime.toNS().count());
      counters.TimeError++;
      continue;
    }
    
    // Check for out_of_range errors thrown by the HashMap2D, which contains
    // the serializers
    try {

      CbmType Type = CbmType(Readout.Type);

      if (Type == CbmType::IBM) {
        counters.IBMReadoutsProcessed++;

        HistogramSerializerMap.get(Readout.FENId, Readout.Channel)
            ->addEvent(TimeOfFlight, Readout.NPos);

        XTRACE(DATA, DEB,
               "CBM Event, CbmType: %" PRIu8 " NPOS: %" PRIu32 " TOF %" PRIu64
               "ns",
               Readout.Type, Readout.NPos, TimeOfFlight);

        counters.IBMEvents++;
      }

      else if (Type == CbmType::EVENT_0D) {
        counters.Event0DReadoutsProcessed++;

        // Register pixels according to the topology map pixel offset
        auto &PixelId = Conf.TopologyMapPtr->get(Readout.FENId, Readout.Channel)
                            ->pixelOffset;

        Ev44SerializerMap.get(Readout.FENId, Readout.Channel)
            ->addEvent(TimeOfFlight, PixelId);

        XTRACE(DATA, DEB,
               "CBM Event, CbmType: %" PRIu8 " Pixel: %" PRIu32 " TOF %" PRIu64
               "ns",
               Readout.Type, PixelId, TimeOfFlight);

        counters.Event0DEvents++;

      } else {
        XTRACE(DATA, WAR, "Type %d currently not supported by EFU",
               Readout.Type);
        counters.TypeNotConfigured++;
        continue;
      }
    } catch (std::out_of_range &e) {
      XTRACE(DATA, WAR, "No serializer configured for FEN {}, Channel {}",
             Readout.FENId, Readout.Channel);

      counters.NoSerializerCfgError++;
      continue;
    } catch (std::invalid_argument &e) {
      XTRACE(DATA, WAR, "Invalid CbmType: {} for readout {} with time {}",
             e.what(), counters.CbmCounts, ReadoutTime.toNS().count());
      counters.TypeNotConfigured++;
      continue;
    }

    counters.CbmCounts++;
  }

  // Update the time statistics
  counters.TimeStats = RefTime.Stats;
}

} // namespace cbm
