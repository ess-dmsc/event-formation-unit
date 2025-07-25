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
#include <common/readout/vmm3/Readout.h>
#include <common/time/TimeString.h>
#include <math.h>
#include <nmx/NMXInstrument.h>
#include <nmx/geometry/NMXGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Nmx {

/// \brief load configuration and calibration files
NMXInstrument::NMXInstrument(struct Counters &counters, BaseSettings &settings,
                             EV44Serializer &serializer,
                             ESSReadout::Parser &essHeaderParser)
    : counters(counters), Settings(settings), Serializer(serializer),
      ESSHeaderParser(essHeaderParser) {

  loadConfigAndCalib();

  essgeom = ESSGeometry(Conf.NMXFileParameters.SizeX,
                        Conf.NMXFileParameters.SizeY, 1, 1);

  // We can now use the settings in Conf
  if (Conf.FileParameters.InstrumentGeometry == "NMX") {
    GeometryInstance = &NMXGeometryInstance;
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

    // Convert from fiberid to ringid
    int Ring = readout.FiberId / 2;
    uint8_t HybridId = readout.VMM >> 1;
    ESSReadout::Hybrid &Hybrid = Conf.getHybrid(Ring, readout.FENId, HybridId);

    if (!Hybrid.Initialised) {
      XTRACE(DATA, ALW,
             "Hybrid for Ring %d, FEN %d, VMM %d not defined in config file",
             Ring, readout.FENId, HybridId);
      counters.HybridMappingErrors++;
      continue;
    }

    uint8_t AsicId = readout.VMM & 0x1;
    uint16_t Offset = Conf.Offset[Ring][readout.FENId][HybridId];
    uint8_t Plane = Conf.Plane[Ring][readout.FENId][HybridId];
    uint8_t Panel = Conf.Panel[Ring][readout.FENId][HybridId];
    bool ReversedChannels =
        Conf.ReversedChannels[Ring][readout.FENId][HybridId];
    uint16_t MinADC = Hybrid.MinADC;

    // VMM3Calibration & Calib = Hybrids[Hybrid].VMMs[Asic];

    uint64_t TimeNS =
        ESSReadout::ESSTime::toNS(readout.TimeHigh, readout.TimeLow).count();
    //   int64_t TDCCorr = Calib.TDCCorr(readout.Channel, readout.TDC);
    //   XTRACE(DATA, DEB, "TimeNS raw %" PRIu64 ", correction %" PRIi64,
    //   TimeNS, TDCCorr);

    //   TimeNS += TDCCorr;
    //   XTRACE(DATA, DEB, "TimeNS corrected %" PRIu64, TimeNS);

    // Only 10 bits of the 16-bit OTADC field is used hence the 0x3ff mask below
    // uint16_t ADC = Calib.ADCCorr(readout.Channel, readout.OTADC & 0x3FF);
    // no calibration yet, so using raw ADC value
    uint16_t ADC = readout.OTADC & 0x3FF;

    if (ADC < MinADC) {
      XTRACE(DATA, INF, "Under MinADC value, got %u, minimum is %u", ADC,
             MinADC);
      counters.MinADC++;
      continue;
    } else {
      // XTRACE(DATA, DEB, "Valid ADC %u, min is %u", ADC, MinADC);
    }

    // XTRACE(DATA, DEB, "ADC calibration from %u to %u", readout.OTADC &
    // 0x3FF, ADC);

    // If the corrected ADC reaches maximum value we count the occurance but
    // use the new value anyway
    // Only possible if calibration takes it over the max value
    // Original value has already been checked against max value
    /// \todo apply calibration and recheck if over max ADC, is this overall max
    /// adc or still vessel/channel specific?

    // Now we add readouts with the calibrated time and adc to the panel
    // builders

    uint16_t Coord = GeometryInstance->coord(readout.Channel, AsicId, Offset,
                                             ReversedChannels);

    // 65535 is used for invalid coordinate value
    if (Coord == Geometry::InvalidCoord) {
      XTRACE(DATA, ERR, "Invalid Coord");
      counters.MappingErrors++;
      continue;
    }

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
        ESSReadout::Hybrid h = Conf.getHybrid(Ring, FENId, HybridId);
        if (h.Initialised) {
          int Panel = Conf.Panel[Ring][FENId][HybridId];
          int Plane = Conf.Plane[Ring][FENId][HybridId];
          CurrentCoordSet = &Coords[Panel][Plane];
          for (int Asic = 0; Asic < 2; Asic++) {
            XTRACE(EVENT, DEB, "Ring %u, Fen %u, Hybrid %u", Ring, FENId,
                   HybridId);
            for (int channel = 0; channel < 64; channel++) {
              int Offset = Conf.Offset[Ring][FENId][HybridId];
              int ReversedChannels =
                  Conf.ReversedChannels[Ring][FENId][HybridId];
              int coord = NMXGeometryInstance.coord(channel, Asic, Offset,
                                                    ReversedChannels);
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
  for (const auto &e : Events) {
    if (e.empty()) {
      XTRACE(EVENT, DEB, "event empty");
      continue;
    }

    if (!e.both_planes()) {
      XTRACE(EVENT, DEB, "Event has no coincidence");
      counters.ClustersNoCoincidence++;
      if (e.ClusterA.empty()) {
        counters.ClustersMatchedYOnly++;
      }

      if (e.ClusterB.empty()) {
        counters.ClustersMatchedXOnly++;
      }
      continue;
    }

    if (Conf.NMXFileParameters.MaxSpanX < e.ClusterA.coordSpan()) {
      XTRACE(EVENT, DEB, "Event spans too far in X direction, %u",
             e.ClusterA.coordSpan());
      counters.ClustersTooLargeSpanX++;
      continue;
    }

    if (Conf.NMXFileParameters.MinSpanX > e.ClusterA.coordSpan()) {
      XTRACE(EVENT, DEB, "Event doesn't span far enough in X direction, %u",
             e.ClusterA.coordSpan());
      counters.ClustersTooSmallSpanX++;
      continue;
    }

    if (Conf.NMXFileParameters.MaxTimeSpan < e.ClusterA.timeSpan()) {
      XTRACE(EVENT, DEB, "Event spans too long a time in X cluster, %u",
             e.ClusterA.timeSpan());
      counters.ClustersTooLargeTimeSpan++;
      continue;
    }

    if (Conf.NMXFileParameters.MaxSpanY < e.ClusterB.coordSpan()) {
      XTRACE(EVENT, DEB, "Event spans too far in Y direction, %u",
             e.ClusterB.coordSpan());
      counters.ClustersTooLargeSpanY++;
      continue;
    }

    if (Conf.NMXFileParameters.MinSpanY > e.ClusterB.coordSpan()) {
      XTRACE(EVENT, DEB, "Event doesn't span far enough in Y direction, %u",
             e.ClusterB.coordSpan());
      counters.ClustersTooSmallSpanY++;
      continue;
    }

    if (Conf.NMXFileParameters.MaxTimeSpan < e.ClusterB.timeSpan()) {
      XTRACE(EVENT, DEB, "Event spans too long a time in Y cluster, %u",
             e.ClusterB.timeSpan());
      counters.ClustersTooLargeTimeSpan++;
      continue;
    }

    counters.EventsMatchedClusters++;
    XTRACE(EVENT, DEB, "Event Valid\n %s", e.to_string({}, true).c_str());

    // Calculate TOF in ns
    uint64_t EventTime = e.timeEnd();

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

    // calculate local x and y using center of mass
    uint16_t x = static_cast<uint16_t>(std::round(e.ClusterA.coordUtpc(false)));
    uint16_t y = static_cast<uint16_t>(std::round(e.ClusterB.coordUtpc(false)));
    auto PixelId = essgeom.pixel2D(x, y);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR, "Bad pixel!: Time: %u TOF: %u, x %u, y %u, pixel %u",
             time, TimeOfFlight, x, y, PixelId);
      counters.PixelErrors++;
      continue;
    }

    XTRACE(EVENT, INF, "Time: %u TOF: %u, x %u, y %u, pixel %u", time,
           TimeOfFlight, x, y, PixelId);
    Serializer.addEvent(TimeOfFlight, PixelId);
    counters.Events++;
  }
  Events.clear(); // else events will accumulate
}
} // namespace Nmx
