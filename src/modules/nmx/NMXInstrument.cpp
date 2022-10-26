// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
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
NMXInstrument::NMXInstrument(struct Counters &counters,
                             BaseSettings & settings,
                             EV42Serializer *serializer)
    : counters(counters), Settings(settings),
      Serializer(serializer) {
  if (!Settings.DumpFilePrefix.empty()) {
    std::string DumpFileName =
        Settings.DumpFilePrefix + "nmx_" + timeString();
    XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
    DumpFile = VMM3::ReadoutFile::create(DumpFileName);
  }

  loadConfigAndCalib();

  essgeom = ESSGeometry(Conf.NMXFileParameters.SizeX,
                        Conf.NMXFileParameters.SizeY, 1, 1);

  // We can now use the settings in Conf
  if (Conf.FileParameters.InstrumentGeometry == "NMX") {
    GeometryInstance = &NMXGeometryInstance;
  } else {
    throw std::runtime_error("Invalid InstrumentGeometry in config file");
  }

  XTRACE(INIT, ALW, "Set EventBuilder timebox to %u ns",
         Conf.FileParameters.TimeBoxNs);
  for (auto &builder : builders) {
    builder.setTimeBox(Conf.FileParameters.TimeBoxNs); // Time boxing
  }

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.FileParameters.MaxPulseTimeNS);
}

void NMXInstrument::loadConfigAndCalib() {
  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  Conf = Config("NMX", Settings.ConfigFile);
  Conf.loadAndApplyConfig();

  // XTRACE(INIT, ALW, "Creating vector of %d builders (one per hybrid)",
  //        Conf.getNumHybrids());
  builders = std::vector<EventBuilder2D>(Conf.NMXFileParameters.NumPanels);

  /// \todo Add calibration processing
  // if (Settings.CalibFile != "") {
  //   XTRACE(INIT, ALW, "Loading and applying calibration file");
  //   Conf.loadAndApplyCalibration(Settings.CalibFile);
  // }
}

void NMXInstrument::processReadouts(void) {
  // All readouts are potentially now valid, but rings and fens
  // could still be outside the configured range, also
  // illegal time intervals can be detected here
  assert(Serializer != nullptr);
  Serializer->pulseTime(ESSReadoutParser.Packet.Time
                            .TimeInNS); /// \todo sometimes PrevPulseTime maybe?

  XTRACE(DATA, DEB, "processReadouts()");
  for (const auto &readout : VMMParser.Result) {
    if (DumpFile) {
      VMMParser.dumpReadoutToFile(readout, ESSReadoutParser, DumpFile);
    }

    XTRACE(DATA, DEB,
           "readout: Phys RingId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.RingId, readout.FENId, readout.VMM, readout.Channel,
           readout.TimeLow);

    uint8_t HybridId = readout.VMM >> 1;
    ESSReadout::Hybrid &Hybrid =
        Conf.getHybrid(readout.RingId, readout.FENId, HybridId);

    if (!Hybrid.Initialised) {
      XTRACE(DATA, WAR,
             "Hybrid for Ring %d, FEN %d, VMM %d not defined in config file",
             readout.RingId, readout.FENId, HybridId);
      counters.HybridMappingErrors++;
      continue;
    }

    // Convert from physical rings to logical rings
    // uint8_t Ring = readout.RingId/2;
    uint8_t AsicId = readout.VMM & 0x1;
    uint16_t Offset = Conf.Offset[readout.RingId][readout.FENId][HybridId];
    uint8_t Plane = Conf.Plane[readout.RingId][readout.FENId][HybridId];
    uint8_t Panel = Conf.Panel[readout.RingId][readout.FENId][HybridId];
    bool ReversedChannels =
        Conf.ReversedChannels[readout.RingId][readout.FENId][HybridId];
    uint16_t MinADC = Hybrid.MinADC;

    //   VMM3Calibration & Calib = Hybrids[Hybrid].VMMs[Asic];

    uint64_t TimeNS =
        ESSReadoutParser.Packet.Time.toNS(readout.TimeHigh, readout.TimeLow);
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
      XTRACE(DATA, ERR, "Under MinADC value, got %u, minimum is %u", ADC,
             MinADC);
      counters.MinADC++;
      continue;
    } else {
      XTRACE(DATA, DEB, "Valid ADC %u, min is %u", ADC, MinADC);
    }

    //   XTRACE(DATA, DEB, "ADC calibration from %u to %u", readout.OTADC &
    //   0x3FF, ADC);

    // If the corrected ADC reaches maximum value we count the occurance but
    // use the new value anyway
    // Only possible if calibration takes it over the max value
    // Original value has already been checked against max value
    /// \todo apply calibration and recheck if over max ADC, is this overall max
    /// adc or still vessel/channel specific?

    //   // Now we add readouts with the calibrated time and adc to the panel
    //   builders

    uint16_t Coord = GeometryInstance->coord(readout.Channel, AsicId, Offset,
                                             ReversedChannels);

    if (Coord ==
        GeometryInstance->InvalidCoord) { // 65535 is invalid xandzCoordinate
      XTRACE(DATA, ERR, "Invalid Coord");
      counters.MappingErrors++;
      continue;
    }

    XTRACE(DATA, DEB, "Coord %u, Channel %u, Panel %u", Coord, readout.Channel,
           Panel);
    builders[Panel].insert({TimeNS, Coord, ADC, Plane});
    XTRACE(DATA, DEB, "inserted to builder");
  }

  for (auto &builder : builders) {
    builder.flush(); // Do matching
  }
}

void NMXInstrument::generateEvents(std::vector<Event> &Events) {
  ESSReadout::ESSTime &TimeRef = ESSReadoutParser.Packet.Time;

  for (const auto &e : Events) {
    if (e.empty()) {
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

    if (Conf.NMXFileParameters.MaxXSpan < e.ClusterA.coord_span()) {
      XTRACE(EVENT, DEB, "Event spans too far in X direction, %u",
             e.ClusterA.coord_span());
      counters.ClustersTooLargeXSpan++;
      continue;
    }

    if (Conf.NMXFileParameters.MaxYSpan < e.ClusterB.coord_span()) {
      XTRACE(EVENT, DEB, "Event spans too far in Y direction, %u",
             e.ClusterB.coord_span());
      counters.ClustersTooLargeYSpan++;
      continue;
    }

    counters.EventsMatchedClusters++;
    XTRACE(EVENT, DEB, "Event Valid\n %s", e.to_string({}, true).c_str());

    // Calculate TOF in ns
    uint64_t EventTime = e.time_end();

    XTRACE(EVENT, DEB, "EventTime %" PRIu64 ", TimeRef %" PRIu64, EventTime,
           TimeRef.TimeInNS);

    if (TimeRef.TimeInNS > EventTime) {
      XTRACE(EVENT, WAR, "Negative TOF!");
      counters.TimeErrors++;
      continue;
    }

    uint64_t TimeOfFlight = EventTime - TimeRef.TimeInNS;

    if (TimeOfFlight > Conf.FileParameters.MaxTOFNS) {
      XTRACE(DATA, WAR, "TOF larger than %u ns", Conf.FileParameters.MaxTOFNS);
      counters.TOFErrors++;
      continue;
    }

    // calculate local x and y using center of mass
    uint16_t x = static_cast<uint16_t>(std::round(e.ClusterA.coord_utpc(false)));
    uint16_t y = static_cast<uint16_t>(std::round(e.ClusterB.coord_utpc(false)));
    auto PixelId = essgeom.pixel2D(x, y);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR, "Bad pixel!: Time: %u TOF: %u, x %u, y %u, pixel %u",
             time, TimeOfFlight, x, y, PixelId);
      counters.PixelErrors++;
      continue;
    }

    XTRACE(EVENT, INF, "Time: %u TOF: %u, x %u, y %u, pixel %u", time,
           TimeOfFlight, x, y, PixelId);
    counters.TxBytes += Serializer->addEvent(TimeOfFlight, PixelId);
    counters.Events++;
  }
  Events.clear(); // else events will accumulate
}
} // namespace Nmx
