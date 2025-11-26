// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Cbm is responsible for readout validation and event formation for
/// the common beam monitor (CBM) instrument
///
//===----------------------------------------------------------------------===//

#include <common/readout/ess/Parser.h>
#include <common/debug/Trace.h>
#include <common/geometry/DetectorGeometry.h>
#include <modules/cbm/CbmInstrument.h>
#include <modules/cbm/CbmTypes.h>
#include <stdexcept>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#define IBM_ADC_MASK 0xFFFFFF

using namespace esstime;
using namespace geometry;
namespace cbm {


/// \brief load configuration and calibration files
CbmInstrument::CbmInstrument(
    struct Counters &Counters, Config &Config,
    const HashMap2D<EV44Serializer> &Ev44serializerMap,
    const HashMap2D<fbserializer::HistogramSerializer<int32_t>>
        &HistogramSerializerMap,
    ESSReadout::Parser &essHeaderParser)

    : counters(Counters), Conf(Config), Ev44SerializerMap(Ev44serializerMap),
      HistogramSerializerMap(HistogramSerializerMap),
      ESSHeaderParser(essHeaderParser) {

  ESSHeaderParser.setMaxPulseTimeDiff(Conf.Parms.MaxPulseTimeDiffNS);
  ESSHeaderParser.Packet.Time.setMaxTOF(Conf.Parms.MaxTOFNS);

  //To handle EVENT_2D monitors the configuration list must be analysed
  //for EVENT_2D data to create the geometry
  std::vector<Topology*> topologies = Conf.TopologyMapPtr->toValuesList();
  for (const auto *item : topologies) {
    if (item->Type == CbmType::EVENT_2D) {
      // Geometry key is created from FEN and Channel where
      // FEN will be most significant Byte and  channel least significant Byte.
      // TopologyMapPtr is basically using the same key but in a different way.
      uint16_t key = (item->FEN << 8) +  item->Channel;
      Geometries.emplace(key, std::make_unique<ESSGeometry>(
        item->width, item->height, 1, 1));
    }
  }
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

    int Ring = DetectorGeometry<cbm::Parser::CbmReadout>::calcRing(Readout.FiberId);
    if (Ring != Conf.Parms.MonitorRing) {
      XTRACE(DATA, WAR, "Invalid ring %u (expect %u) for monitor readout", Ring,
             Conf.Parms.MonitorRing);
      counters.RingCfgError++;
      continue;
    }

    ESSTime ReadoutTime = ESSTime(Readout.TimeHigh, Readout.TimeLow);

    /// Calculates the time of flight (TOF) for the readout based on the
    /// reference time and the readout time. If the readout time is smaller than
    /// the reference time, it means that tof would be negative and this
    /// function returns the TOF according to the previous reference time. This
    /// is includes delayed readouts into the current pulse statistics. \note:
    /// If time calculation fails, the function returns InvalidTOF
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

        // Get normalized ADC value from readout.
        uint32_t normADC{Readout.NADC.getNADC()};
        if (Readout.NADC.MCASum != 0) {
          normADC /= Readout.NADC.MCASum;
        }

        HistogramSerializerMap.get(Readout.FENId, Readout.Channel)
            ->addEvent(TimeOfFlight, normADC);

        XTRACE(DATA, DEB,
               "CBM Event, CbmType: %" PRIu8 " ADC: %" PRIu32 " MCASum: %" PRIu32 " TOF %" PRIu64
               "ns",
               Readout.Type, Readout.NADC.getNADC(), Readout.NADC.MCASum, TimeOfFlight);

        counters.IBMEvents++;
        counters.NPOSCount += normADC;

      } else if (Type == CbmType::EVENT_0D) {
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
      } else if (Type == CbmType::EVENT_2D) {
        counters.Event2DReadoutsProcessed++;

        auto *topology = Conf.TopologyMapPtr->get(Readout.FENId, Readout.Channel);
        if ((Readout.Pos.XPos > topology->width) || (Readout.Pos.YPos > topology->height)) {
          CbmReadoutParser.Stats.ErrorADC++;

          XTRACE(DATA, DEB,
                "CBM Event, CbmType: %" PRIu32 " TOF %" PRIu64
                "ns, XPos %" PRIu32 " YPos %" PRIu32,
                Readout.Type, TimeOfFlight, Readout.Pos.XPos, Readout.Pos.YPos);
        } else {
          const auto &geometry = Geometries.at((Readout.FENId << 8) + Readout.Channel);
          uint32_t PixelId = geometry->pixel2D(Readout.Pos.XPos, Readout.Pos.YPos);

          Ev44SerializerMap.get(Readout.FENId, Readout.Channel)
              ->addEvent(TimeOfFlight, PixelId);

          XTRACE(DATA, DEB,
                "CBM Event, CbmType: %" PRIu8 " Pixel: %" PRIu32 " TOF %" PRIu64
                "ns, XPos %" PRIu32 " YPos %" PRIu32,
                Readout.Type, PixelId, TimeOfFlight, Readout.Pos.XPos, Readout.Pos.YPos);
        }

        counters.Event2DEvents++;
      } else {
        XTRACE(DATA, WAR, "Type %d currently not supported by EFU",
               Readout.Type);
        counters.TypeNotConfigured++;
        continue;
      }
    } catch (std::out_of_range &e) {
      XTRACE(DATA, WAR, "No serializer configured for FEN %" PRIu8 ", Channel %" PRIu8,
             Readout.FENId, Readout.Channel);

      counters.NoSerializerCfgError++;
      continue;
    } catch (std::invalid_argument &e) {
      XTRACE(DATA, WAR, "Invalid CbmType: %" PRIu8 " for readout %" PRIu8 " with time %" PRIu8 ,
             e.what(), counters.CbmCounts, ReadoutTime.toNS().count());
      counters.TypeNotConfigured++;
      continue;
    }

    counters.CbmCounts++;
  }
}

} // namespace cbm
