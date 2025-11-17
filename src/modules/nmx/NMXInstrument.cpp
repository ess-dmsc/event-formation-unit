// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMXInstrument is responsible for readout validation and event
/// formation
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/geometry/DetectorGeometry.h>
#include <common/geometry/vmm3/VMM3Geometry.h>
#include <common/readout/vmm3/Readout.h>
#include <common/time/TimeString.h>
#include <cstdint>
#include <math.h>
#include <nmx/NMXInstrument.h>
#include <nmx/geometry/NMXGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace vmm3;
using namespace geometry;

namespace Nmx {

/// \brief load configuration and calibration files
NMXInstrument::NMXInstrument(struct Counters &counters, BaseSettings &settings,
                             EV44Serializer &serializer,
                             ESSReadout::Parser &essHeaderParser,
                             Statistics &stats)
    : counters(counters), Settings(settings), Stats(stats),
      Serializer(serializer), ESSHeaderParser(essHeaderParser) {

  loadConfigAndCalib();

  // We can now use the settings in Conf
  if (Conf.FileParameters.InstrumentGeometry == "NMX") {
    // Create geometry instance with proper inheritance
    NMXGeom =
        std::make_unique<NMXGeometry>(Stats, Conf, Conf.MaxRing, Conf.MaxFEN);
  } else {
    throw std::runtime_error("Invalid InstrumentGeometry in config file");
  }

  ESSHeaderParser.setMaxPulseTimeDiff(Conf.FileParameters.MaxPulseTimeNS);
}

void NMXInstrument::loadConfigAndCalib() {
  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  Conf = Config("NMX", Settings.ConfigFile);
  Conf.loadAndApplyConfig();
  checkConfigAndGeometry();

  builders = std::vector<EventBuilder2D>(Conf.NMXFileParameters.NumPanels);
  for (EventBuilder2D &builder : builders) {
    builder.matcher.setMaximumTimeGap(
        Conf.NMXFileParameters.MaxMatchingTimeGap);
    builder.ClustererX.setMaximumTimeGap(
        Conf.NMXFileParameters.MaxClusteringTimeGap);
    builder.ClustererY.setMaximumTimeGap(
        Conf.NMXFileParameters.MaxClusteringTimeGap);
    if (Conf.NMXFileParameters.SplitMultiEvents) {
      builder.matcher.setSplitMultiEvents(
          Conf.NMXFileParameters.SplitMultiEvents,
          Conf.NMXFileParameters.SplitMultiEventsCoefficientLow,
          Conf.NMXFileParameters.SplitMultiEventsCoefficientHigh);
    }
  }

  if (Settings.CalibFile != "") {
    XTRACE(INIT, ALW, "Loading and applying calibration file %s",
           Settings.CalibFile.c_str());
    Conf.loadAndApplyCalibration(Settings.CalibFile);
  }
}

void NMXInstrument::processReadouts() {
  // All readouts are potentially now valid, but rings and fens
  // could still be outside the configured range, also
  // illegal time intervals can be detected here
  /// \todo sometimes PrevPulseTime maybe?
  Serializer.checkAndSetReferenceTime(
      ESSHeaderParser.Packet.Time.getRefTimeUInt64());
  XTRACE(DATA, DEB, "processReadouts()");
  for (const auto &readout : VMMParser.Result) {

    XTRACE(DATA, DEB,
           "readout: FiberId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.FiberId, readout.FENId, readout.VMM, readout.Channel,
           readout.TimeLow);

    // Validate readout data using Freia-style validation
    if (!NMXGeom->validateReadoutData(readout)) {
      continue;
    }

    // Convert from fiberid to ringid
    int Ring = DetectorGeometry::calcRing(readout.FiberId);
    uint8_t HybridId = VMM3Geometry::calcHybridId(readout.VMM);

    // CACHE-OPTIMIZED: Single 3D array lookup instead of 4 separate lookups
    // This loads all 4 fields from adjacent memory locations in one cache line
    const auto &params = Conf.HybridParam[Ring][readout.FENId][HybridId];

    // Extract individual fields from the cached struct
    const uint16_t Offset = params.Offset;
    const uint8_t Plane = params.Plane;
    const uint8_t Panel = params.Panel;
    const bool ReversedChannels = params.ReversedChannels;

    // VMM3Calibration & Calib = Hybrids[Hybrid].VMMs[Asic];

    uint64_t TimeNS =
        ESSReadout::ESSTime::toNS(readout.TimeHigh, readout.TimeLow).count();
    //   int64_t TDCCorr = Calib.TDCCorr(readout.Channel, readout.TDC);
    //   XTRACE(DATA, DEB, "TimeNS raw %" PRIu64 ", correction %" PRIi64,
    //   TimeNS, TDCCorr);

    //   TimeNS += TDCCorr;
    //   XTRACE(DATA, DEB, "TimeNS corrected %" PRIu64, TimeNS);

    uint16_t Coord =
        NMXGeom->coord(readout.Channel, NMXGeom->getAsicId(readout.VMM), Offset,
                       ReversedChannels);

    // 65535 is used for invalid coordinate value
    if (Coord == NMXGeometry::InvalidCoord) {
      XTRACE(DATA, ERR, "Invalid Coord");
      counters.MappingErrors++;
      continue;
    }

    uint16_t ADC = VMM3Geometry::getRawADC(readout.OTADC);

    XTRACE(DATA, DEB, "Plane %u, Coord %u, Channel %u, Panel %u", Plane, Coord,
           readout.Channel, Panel);
    builders[Panel].insert({TimeNS, Coord, ADC, Plane});
  }

  for (auto &builder : builders) {
    builder.flush(true); // Do matching, and flush the matcher
  }
}

void NMXInstrument::checkConfigAndGeometry() {
  std::set<int> Coords[4][2];
  std::set<int> *CurrentCoordSet;

  for (int Ring = 0; Ring <= Conf.MaxRing; Ring++) {
    for (int FENId = 0; FENId <= Conf.MaxFEN; FENId++) {
      for (int HybridId = 0; HybridId <= Conf.MaxHybrid; HybridId++) {
        Hybrid h = Conf.getHybrid(Ring, FENId, HybridId);
        if (h.Initialised) {
          // CACHE-OPTIMIZED: Single lookup for all hybrid parameters
          const auto &params = Conf.HybridParam[Ring][FENId][HybridId];
          CurrentCoordSet = &Coords[params.Panel][params.Plane];

          for (int Asic = 0; Asic < 2; Asic++) {
            XTRACE(EVENT, DEB, "Ring %u, Fen %u, Hybrid %u", Ring, FENId,
                   HybridId);
            for (int channel = 0; channel < 64; channel++) {
              // Use cached params instead of multiple 3D array lookups
              int coord = NMXGeom->coord(channel, Asic, params.Offset,
                                         params.ReversedChannels);
              if (CurrentCoordSet->count(coord)) {
                XTRACE(INIT, ERR,
                       "Channel %u, Coordinate %u already covered by another "
                       "hybrid",
                       channel, coord);
                throw std::runtime_error("Invalid config, coordinates overlap");
              } else {
                CurrentCoordSet->insert(coord);
              }
            }
          }
        }
      }
    }
  }
}

void NMXInstrument::generateEvents(std::vector<Event> &Events) {
  XTRACE(EVENT, DEB, "generateEvents()");
  ESSReadout::ESSReferenceTime &TimeRef = ESSHeaderParser.Packet.Time;
  for (const auto &Event : Events) {
    if (Event.empty()) {
      XTRACE(EVENT, DEB, "event empty");
      continue;
    }

    if (!Event.both_planes()) {
      XTRACE(EVENT, DEB, "Event has no coincidence");
      counters.ClustersNoCoincidence++;
      if (Event.ClusterA.empty()) {
        counters.ClustersMatchedYOnly++;
      }

      if (Event.ClusterB.empty()) {
        counters.ClustersMatchedXOnly++;
      }
      continue;
    }

    if (Conf.NMXFileParameters.MaxSpanX < Event.ClusterA.coordSpan()) {
      XTRACE(EVENT, DEB, "Event spans too far in X direction, %u",
             Event.ClusterA.coordSpan());
      counters.ClustersTooLargeSpanX++;
      continue;
    }

    if (Conf.NMXFileParameters.MinSpanX > Event.ClusterA.coordSpan()) {
      XTRACE(EVENT, DEB, "Event doesn't span far enough in X direction, %u",
             Event.ClusterA.coordSpan());
      counters.ClustersTooSmallSpanX++;
      continue;
    }

    if (Conf.NMXFileParameters.MaxTimeSpan < Event.ClusterA.timeSpan()) {
      XTRACE(EVENT, DEB, "Event spans too long a time in X cluster, %u",
             Event.ClusterA.timeSpan());
      counters.ClustersTooLargeTimeSpan++;
      continue;
    }

    if (Conf.NMXFileParameters.MaxSpanY < Event.ClusterB.coordSpan()) {
      XTRACE(EVENT, DEB, "Event spans too far in Y direction, %u",
             Event.ClusterB.coordSpan());
      counters.ClustersTooLargeSpanY++;
      continue;
    }

    if (Conf.NMXFileParameters.MinSpanY > Event.ClusterB.coordSpan()) {
      XTRACE(EVENT, DEB, "Event doesn't span far enough in Y direction, %u",
             Event.ClusterB.coordSpan());
      counters.ClustersTooSmallSpanY++;
      continue;
    }

    if (Conf.NMXFileParameters.MaxTimeSpan < Event.ClusterB.timeSpan()) {
      XTRACE(EVENT, DEB, "Event spans too long a time in Y cluster, %u",
             Event.ClusterB.timeSpan());
      counters.ClustersTooLargeTimeSpan++;
      continue;
    }

    counters.EventsMatchedClusters++;
    XTRACE(EVENT, DEB, "Event Valid\n %s", Event.to_string({}, true).c_str());

    // Calculate TOF in ns
    uint64_t EventTime = Event.timeEnd();

    XTRACE(EVENT, DEB, "EventTime %" PRIu64 ", TimeRef %" PRIu64, EventTime,
           TimeRef.getRefTimeUInt64());

    if (TimeRef.getRefTimeUInt64() > EventTime) {
      XTRACE(EVENT, WAR, "Negative TOF!");
      counters.TimeErrors++;
      continue;
    }

    uint64_t TimeOfFlight = EventTime - TimeRef.getRefTimeUInt64();

    if (TimeOfFlight > Conf.FileParameters.MaxTOFNS) {
      XTRACE(DATA, WAR, "TOF larger than %u ns", Conf.FileParameters.MaxTOFNS);
      counters.TOFErrors++;
      continue;
    }

    auto PixelId = NMXGeom->calcPixel(Event);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR, "Bad pixel!: Time: %u TOF: %u, pixel %u", time,
             TimeOfFlight, PixelId);
      continue;
    }

    XTRACE(EVENT, INF, "Time: %u TOF: %u, pixel %u", time, TimeOfFlight,
           PixelId);

    Serializer.addEvent(TimeOfFlight, PixelId);

    counters.Events++;
  }
  Events.clear(); // else events will accumulate
}
} // namespace Nmx
