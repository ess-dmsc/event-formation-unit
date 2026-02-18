// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
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
#include <modules/cbm/geometry/Geometry0D.h>
#include <modules/cbm/geometry/Geometry2D.h>
#include <stdexcept>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace esstime;
using namespace geometry;
namespace cbm {

/// \brief load configuration and calibration files
CbmInstrument::CbmInstrument(Statistics &Stats, struct Counters &Counters,
                             Config &Config, Parser &cbmReadoutParser,
                             const HashMap2D<SchemaDetails> &SchemaDetailMap,
                             ESSReadout::Parser &essHeaderParser)
    : CbmReadoutParser(cbmReadoutParser), counters(Counters), Conf(Config),
      SchemaMap(SchemaDetailMap), ESSHeaderParser(essHeaderParser) {

  ESSHeaderParser.setMaxPulseTimeDiff(Conf.CbmParms.MaxPulseTimeDiffNS);
  ESSHeaderParser.Packet.Time.setMaxTOF(Conf.CbmParms.MaxTOFNS);

  // Analyze topology configuration to create appropriate geometries for each
  // monitor type. Geometry key is created from FEN and Channel where
  // FEN will be most significant Byte and channel least significant Byte.
  std::vector<Topology *> topologies = Conf.TopologyMapPtr->toValuesList();
  for (const auto *item : topologies) {
    uint16_t key = calcGeometryKey(item->FEN, item->Channel);

    if (item->Type == CbmType::EVENT_2D) {

      // EVENT_2D monitors need 2D geometry with X/Y validation
      Geometries.emplace(key, std::make_unique<Geometry2D>(
                                  Stats, Conf, item->Type, item->Source,
                                  item->width, item->height));
    } else if (item->Type == CbmType::EVENT_0D) {

      // EVENT_0D monitors use base geometry with fixed pixel offset
      Geometries.emplace(key, std::make_unique<Geometry0D>(
                                  Stats, Conf, item->Type, item->Source,
                                  static_cast<uint32_t>(item->pixelOffset)));
    } else if (item->Type == CbmType::IBM) {

      // IBM monitors use base geometry with pixel offset 0 (not used for
      // histograms)
      Geometries.emplace(key, std::make_unique<Geometry0D>(
                                  Stats, Conf, item->Type, item->Source, 0));
    }
  }
}

void CbmInstrument::processMonitorReadouts() {
  ESSReadout::ESSReferenceTime &RefTime = ESSHeaderParser.Packet.Time;
  // All readouts are now potentially valid, negative TOF is not
  // possible, or 0 ADC values, but rings and fens could still be outside the
  // configured range, also illegal time intervals can be detected here

  for (auto &Details : SchemaMap.toValuesList()) {
    if (Details->GetSchema() == SchemaType::DA00) {
      DA00Serializer_t *Serializer = Details->GetSerializer<SchemaType::DA00>();
      Serializer->checkAndSetReferenceTime(
          ESSHeaderParser.Packet.Time.getRefTimeNS());
      /// \todo sometimes PrevPulseTime maybe?
    } else {
      EV44Serializer_t *Serializer = Details->GetSerializer<SchemaType::EV44>();
      Serializer->checkAndSetReferenceTime(
          ESSHeaderParser.Packet.Time.getRefTimeUInt64());
      /// \todo sometimes PrevPulseTime maybe?
    }
  }

  XTRACE(DATA, DEB, "processMonitorReadouts() - has %zu entries",
         CbmReadoutParser.Result.size());

  for (const auto &Readout : CbmReadoutParser.Result) {

    XTRACE(DATA, DEB,
           "readout: FiberId %d, FENId %d, POS %d, Type %d, Channel %d, ADC "
           "%d, TimeHigh %d, TimeLow %d",
           Readout.FiberId, Readout.FENId, Readout.Pos, Readout.Type,
           Readout.Channel, Readout.ADC, Readout.TimeHigh, Readout.TimeLow);

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

        // Get geometry for validation
        const auto &geometry =
            Geometries.at(calcGeometryKey(Readout.FENId, Readout.Channel));

        // Validate readout data (Ring/FEN/Topology)
        if (!geometry->validateReadoutData(Readout)) {
          XTRACE(DATA, WAR, "Invalid IBM readout data, skipping");
          continue;
        }

        // Get normalized ADC value from readout.
        uint32_t normADC{Readout.NADC.getNADC()};
        if (Conf.CbmParms.NormalizeIBMReadouts && (Readout.NADC.MCASum != 0)) {
          normADC /= Readout.NADC.MCASum;
        }
        const SchemaDetails *Details =
            SchemaMap.get(Readout.FENId, Readout.Channel);
        if (Details->GetSchema() == SchemaType::DA00) {
          Details->GetSerializer<SchemaType::DA00>()->addEvent(TimeOfFlight,
                                                               normADC);
        } else {
          Details->GetSerializer<SchemaType::EV44>()->addEvent(TimeOfFlight,
                                                               normADC);
        }

        XTRACE(DATA, DEB,
               "CBM IBM Event, CbmType: %" PRIu8 " ADC: %" PRIu32
               " MCASum: %" PRIu32 " TOF %" PRIu64 "ns",
               Readout.Type, Readout.NADC.getNADC(), Readout.NADC.MCASum,
               TimeOfFlight);

        counters.IBMEvents++;
        counters.NPOSCount += normADC;

      } else if (Type == CbmType::EVENT_0D) {

        counters.Event0DReadoutsProcessed++;

        // Get geometry for validation and pixel calculation
        const auto &geometry =
            Geometries.at(calcGeometryKey(Readout.FENId, Readout.Channel));

        // Validate readout data (Ring/FEN/Topology)
        if (!geometry->validateReadoutData(Readout)) {
          XTRACE(DATA, WAR, "Invalid EVENT_0D readout data, skipping");
          continue;
        }

        // Get pixel from geometry (fixed offset configured in topology)
        uint32_t PixelId = geometry->calcPixel(Readout);

        SchemaDetails *Details = SchemaMap.get(Readout.FENId, Readout.Channel);
        if (Details->GetSchema() == SchemaType::DA00) {
          Details->GetSerializer<SchemaType::DA00>()->addEvent(TimeOfFlight, 1);
        } else {
          Details->GetSerializer<SchemaType::EV44>()->addEvent(TimeOfFlight,
                                                               PixelId);
        }

        XTRACE(DATA, DEB,
               "CBM 0D Event, CbmType: %" PRIu8 " Pixel: %" PRIu32
               " TOF %" PRIu64 "ns",
               Readout.Type, PixelId, TimeOfFlight);

        counters.Event0DEvents++;

      } else if (Type == CbmType::EVENT_2D) {

        counters.Event2DReadoutsProcessed++;

        const auto &geometry =
            Geometries.at(calcGeometryKey(Readout.FENId, Readout.Channel));

        // Validate readout data before pixel calculation
        bool isValid = geometry->validateReadoutData(Readout);
        if (!isValid) {
          XTRACE(DATA, WAR, "Invalid EVENT_2D data, skipping readout");
          continue;
        }

        // Calculate pixel using template wrapper with automatic error counting
        uint32_t PixelId = geometry->calcPixel(Readout);

        if (PixelId == 0) {
          // Pixel calculation failed - error already counted by Geometry2D
          XTRACE(DATA, DEB,
                 "CBM Event pixel calculation failed, CbmType: %" PRIu8
                 " TOF %" PRIu64 "ns, XPos %" PRIu16 " YPos %" PRIu16,
                 Readout.Type, TimeOfFlight, Readout.Pos.XPos,
                 Readout.Pos.YPos);
        } else {

          SchemaMap.get(Readout.FENId, Readout.Channel)
              ->GetSerializer<SchemaType::EV44>()
              ->addEvent(TimeOfFlight, PixelId);

          counters.Event2DEvents++;

          XTRACE(DATA, DEB,
                 "CBM 2D Event, CbmType: %" PRIu8 " Pixel: %" PRIu32
                 " TOF %" PRIu64 "ns, XPos %" PRIu16 " YPos %" PRIu16,
                 Readout.Type, PixelId, TimeOfFlight, Readout.Pos.XPos,
                 Readout.Pos.YPos);
        }
      } else {
        XTRACE(DATA, WAR, "Type %d currently not supported by EFU",
               Readout.Type);
        counters.TypeNotConfigured++;
        continue;
      }
    } catch (std::out_of_range &e) {
      XTRACE(DATA, WAR,
             "No serializer configured for FEN %" PRIu8 ", Channel %" PRIu8,
             Readout.FENId, Readout.Channel);

      counters.NoSerializerCfgError++;
      continue;
    } catch (std::invalid_argument &e) {
      XTRACE(DATA, WAR,
             "Invalid CbmType: %" PRIu8 " for readout %" PRIu8
             " with time %" PRIu8,
             e.what(), counters.CbmCounts, ReadoutTime.toNS().count());
      counters.TypeNotConfigured++;
      continue;
    }

    counters.CbmCounts++;
  }
}

} // namespace cbm
