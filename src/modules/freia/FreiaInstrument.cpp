// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief FreiaInstrument is responsible for readout validation and event
/// formation
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/Readout.h>
#include <common/time/TimeString.h>
#include <freia/FreiaInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

/// \brief load configuration and calibration files
FreiaInstrument::FreiaInstrument(struct Counters &counters,
                                 // BaseSettings & EFUSettings,
                                 FreiaSettings &moduleSettings,
                                 EV42Serializer *serializer)
    : counters(counters), ModuleSettings(moduleSettings),
      Serializer(serializer) {

  if (!ModuleSettings.FilePrefix.empty()) {
    std::string DumpFileName =
        ModuleSettings.FilePrefix + "freia_" + timeString();
    XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
    DumpFile = VMM3::ReadoutFile::create(DumpFileName);
  }

  loadConfigAndCalib();

  // We can now use the settings in Conf

  Geom.setGeometry(Conf.FileParameters.InstrumentGeometry);

  XTRACE(INIT, ALW, "Set EventBuilder timebox to %u ns",
         Conf.FileParameters.TimeBoxNs);
  for (auto &builder : builders) {
    builder.setTimeBox(Conf.FileParameters.TimeBoxNs); // Time boxing
  }

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.FileParameters.MaxPulseTimeNS);

  // Reinit histogram size (was set to 1 in class definition)
  // ADC is 10 bit 2^10 = 1024
  // Each plane (x,y) has a maximum of NumCassettes * 64 channels
  // eventhough there are only 32 wires so some bins will be empty
  // Hists will automatically allocate space for both x and y planes
  uint32_t MaxADC = 1024;
  uint32_t MaxChannels = Conf.NumHybrids * std::max(GeometryBase::NumWires,
                                                    GeometryBase::NumStrips);
  ADCHist = Hists(MaxChannels, MaxADC);
}

void FreiaInstrument::loadConfigAndCalib() {
  XTRACE(INIT, ALW, "Loading configuration file %s",
         ModuleSettings.ConfigFile.c_str());
  Conf = Config("Freia", ModuleSettings.ConfigFile);
  Conf.loadAndApplyConfig();

  XTRACE(INIT, ALW, "Creating vector of %d builders (one per cassette/hybrid)",
         Conf.NumHybrids);
  builders = std::vector<EventBuilder2D>(Conf.NumHybrids);

  if (ModuleSettings.CalibFile != "") {
    XTRACE(INIT, ALW, "Loading and applying calibration file");
    Conf.loadAndApplyCalibration(ModuleSettings.CalibFile);
  }
}

void FreiaInstrument::setHybridIds(std::vector<std::string> Ids) {
  if (Ids.size() != Conf.NumHybrids) {
    throw std::runtime_error("Hybrid Id size mismatch");
  }
  for (uint8_t i = 0; i < Ids.size(); i++) {
    if (not ESSReadout::Hybrid::isAvailable(Ids[i], Hybrids)) {
      XTRACE(INIT, ERR, "Duplicate Hybrid ID: %s", Ids[i].c_str());
      throw std::runtime_error("Duplicate Hybrid ID");
    }
    Hybrids[i].HybridId = Ids[i];
    XTRACE(INIT, ALW, "Config: Hybrid %u has ID: %s", i, Ids[i].c_str());
  }
}

void FreiaInstrument::processReadouts(void) {
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

    XTRACE(DATA, INF,
           "readout: Phys RingId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.RingId, readout.FENId, readout.VMM, readout.Channel,
           readout.TimeLow);

    // counters.RingRx[readout.RingId]++;

    // Convert from physical rings to logical rings
    uint8_t Ring = readout.RingId / 2;

    // Check for configuration mismatch
    if (Ring > VMM3Config::MaxRing) {
      counters.RingMappingErrors++;
      continue;
    }

    if (readout.FENId > VMM3Config::MaxFEN) {
      counters.FENMappingErrors++;
      continue;
    }

    uint8_t HybridId = readout.VMM >> 1;
    if (!Conf.getHybrid(Ring, readout.FENId, HybridId).Initialised) {
      XTRACE(DATA, WAR,
             "Hybrid for Ring %d, FEN %d, VMM %d not defined in config file",
             Ring, readout.FENId, HybridId);
      counters.HybridMappingErrors++;
      continue;
    }

    ESSReadout::Hybrid &Hybrid =
        Conf.getHybrid(Ring, readout.FENId, readout.VMM >> 1);

    uint8_t Asic = readout.VMM & 0x1;
    XTRACE(DATA, DEB, "Asic calculated to be %u", Asic);
    VMM3Calibration &Calib = Hybrid.VMMs[Asic];
    XTRACE(DATA, DEB, "Hybrid at: %p", &Hybrid);
    XTRACE(DATA, DEB, "Calibration at: %p", &Hybrid.VMMs[Asic]);

    uint64_t TimeNS =
        ESSReadoutParser.Packet.Time.toNS(readout.TimeHigh, readout.TimeLow);
    int64_t TDCCorr = Calib.TDCCorr(readout.Channel, readout.TDC);
    XTRACE(DATA, DEB, "TimeNS raw %" PRIu64 ", correction %" PRIi64, TimeNS,
           TDCCorr);

    TimeNS += TDCCorr;
    XTRACE(DATA, DEB, "TimeNS corrected %" PRIu64, TimeNS);

    // Only 10 bits of the 16-bit OTADC field is used hence the 0x3ff mask below
    uint16_t ADC = Calib.ADCCorr(readout.Channel, readout.OTADC & 0x3FF);

    XTRACE(DATA, DEB, "ADC calibration from %u to %u", readout.OTADC & 0x3FF,
           ADC);

    // If the corrected ADC reaches maximum value we count the occurance but
    // use the new value anyway
    if (ADC >= 1023) {
      counters.MaxADC++;
    }

    // Now we add readouts with the calibrated time and adc to the x,y builders
    if (Geom.isXCoord(readout.VMM)) {
      XTRACE(DATA, DEB,
             "X: TimeNS %" PRIu64 ", Plane %u, Coord %u, Channel %u, ADC %u",
             TimeNS, PlaneX, Geom.xCoord(readout.VMM, readout.Channel),
             readout.Channel, ADC);
      builders[Hybrid.HybridNumber].insert(
          {TimeNS, Geom.xCoord(readout.VMM, readout.Channel), ADC, PlaneX});

      uint32_t GlobalXChannel = Hybrid.XOffset + readout.Channel;
      ADCHist.bin_x(GlobalXChannel, ADC);

    } else { // implicit isYCoord
      XTRACE(DATA, DEB,
             "Y: TimeNS %" PRIu64 ", Plane %u, Coord %u, Channel %u, ADC %u",
             TimeNS, PlaneY,
             Geom.yCoord(Hybrid.YOffset, readout.VMM, readout.Channel),
             readout.Channel, ADC);
      builders[Hybrid.HybridNumber].insert(
          {TimeNS, Geom.yCoord(Hybrid.YOffset, readout.VMM, readout.Channel),
           ADC, PlaneY});

      uint32_t GlobalYChannel = Hybrid.YOffset + readout.Channel;
      ADCHist.bin_y(GlobalYChannel, ADC);
    }
  }

  for (auto &builder : builders) {
    builder.flush(); // Do matching
  }
}

void FreiaInstrument::generateEvents(std::vector<Event> &Events) {
  ESSReadout::ESSTime &TimeRef = ESSReadoutParser.Packet.Time;
  XTRACE(EVENT, DEB, "Number of events: %u", Events.size());
  for (const auto &e : Events) {
    if (e.empty()) {
      XTRACE(EVENT, DEB, "Empty event");
      continue;
    }

    if (!e.both_planes()) {
      XTRACE(EVENT, DEB, "Event has no coincidence");
      counters.EventsNoCoincidence++;

      if (not e.ClusterB.empty()) {
        counters.EventsMatchedWireOnly++;
      }

      if (not e.ClusterA.empty()) {
        counters.EventsMatchedStripOnly++;
      }
      continue;
    }

    // Discard if there are gaps in the strip or wire channels
    if (Conf.WireGapCheck) {
      if (e.ClusterB.hasGap(Conf.MaxGapWire)) {
        XTRACE(EVENT, DEB, "Event discarded due to wire gap");
        counters.EventsInvalidWireGap++;
        continue;
      }
    }

    if (Conf.StripGapCheck) {
      if (e.ClusterA.hasGap(Conf.MaxGapStrip)) {
        XTRACE(EVENT, DEB, "Event discarded due to strip gap");
        counters.EventsInvalidStripGap++;
        continue;
      }
    }

    counters.EventsMatchedClusters++;
    XTRACE(EVENT, DEB, "Event Valid\n %s", e.to_string({}, true).c_str());

    // Calculate TOF in ns
    uint64_t EventTime = e.time_start();

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
      counters.MaxTOFErrors++;
      continue;
    }

    // calculate local x and y using center of mass
    auto x = static_cast<uint16_t>(std::round(e.ClusterA.coord_center()));
    auto y = static_cast<uint16_t>(std::round(e.ClusterB.coord_center()));
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

} // namespace Freia
