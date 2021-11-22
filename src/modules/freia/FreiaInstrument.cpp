// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief FreiaInstrument is responsible for readout validation and event
/// formation
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <freia/FreiaInstrument.h>
#include <common/readout/vmm3/CalibFile.h>
#include <common/readout/vmm3/Readout.h>
#include <assert.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

/// \brief load configuration and calibration files
FreiaInstrument::FreiaInstrument(struct Counters & counters,
  //BaseSettings & EFUSettings,
  FreiaSettings &moduleSettings,
  EV42Serializer * serializer)
    : counters(counters)
    , ModuleSettings(moduleSettings)
    , Serializer(serializer) {

  if (!ModuleSettings.FilePrefix.empty()) {
    std::string DumpFileName = ModuleSettings.FilePrefix + "freia_" + timeString();
    XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
    DumpFile = VMM3::ReadoutFile::create(DumpFileName);
  }

  loadConfigAndCalib();

  // We can now use the settings in Conf.Parms

  Geom.setGeometry(Conf.Parms.InstrumentGeometry);

  XTRACE(INIT, ALW, "Set EventBuilder timebox to %u ns", Conf.Parms.TimeBoxNs);
  for (auto & builder : builders) {
    builder.setTimeBox(Conf.Parms.TimeBoxNs); // Time boxing
  }

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.Parms.MaxPulseTimeNS);

  // Reinit histogram size (was set to 1 in class definition)
  // ADC is 10 bit 2^10 = 1024
  // Each plane (x,y) has a maximum of NumCassettes * 64 channels
  // Hists will automatically allocate space for both x and y planes
  ADCHist = Hists(Conf.NumHybrids * 64, 1024); // 10 bit ADC
  TDCHist = Hists(Conf.NumHybrids * 64, 4096); // 12 bit TDC
}


void FreiaInstrument::loadConfigAndCalib() {
  XTRACE(INIT, ALW, "Loading configuration file %s",
         ModuleSettings.ConfigFile.c_str());
  Conf = Config("Freia", ModuleSettings.ConfigFile);
  Conf.loadAndApply();

  XTRACE(INIT, ALW, "Creating vector of %d builders (one per cassette/hybrid)",
         Conf.NumHybrids);
  builders = std::vector<EventBuilder>(Conf.NumHybrids);

  XTRACE(INIT, ALW, "Creating vector of %d Hybrids (one per cassette/hybrid)",
         Conf.NumHybrids);
  Hybrids = std::vector<ESSReadout::Hybrid>(Conf.NumHybrids);
  XTRACE(INIT, ALW, "Set individual HybridIDs");
  setHybridIds(Conf.HybridStr);


  if (ModuleSettings.CalibFile != "") {
    XTRACE(INIT, ALW, "Loading and applying calibration file");
    ESSReadout::CalibFile Calibration("Freia", Hybrids);
    Calibration.load(ModuleSettings.CalibFile);
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
  Serializer->pulseTime(ESSReadoutParser.Packet.Time.TimeInNS); /// \todo sometimes PrevPulseTime maybe?

  XTRACE(DATA, DEB, "processReadouts()");
  for (const auto & readout : VMMParser.Result) {

    if (DumpFile) {
      dumpReadoutToFile(readout);
    }

    XTRACE(DATA, DEB, "readout: Phys RingId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.RingId, readout.FENId, readout.VMM, readout.Channel, readout.TimeLow);

    //counters.RingRx[readout.RingId]++;

    // Convert from physical rings to logical rings
    uint8_t Ring = readout.RingId/2;

    if (Conf.NumFENs[Ring] == 0) {
      XTRACE(DATA, WAR, "No FENs on RingId %d (physical %d)",
             Ring, readout.RingId);
      counters.RingErrors++;
      continue;
    }

    if (readout.FENId > Conf.NumFENs[Ring]) {
      XTRACE(DATA, WAR, "Invalid FEN %d (max is %d)",
             readout.FENId, Conf.NumFENs[Ring]);
      counters.FENErrors++;
      continue;
    }

    uint8_t Asic = readout.VMM & 0x1;
    uint8_t Hybrid = Conf.getHybridId(Ring, readout.FENId - 1, readout.VMM >> 1);
    uint8_t Cassette = Hybrid + 1;
    VMM3Calibration & Calib = Hybrids[Hybrid].VMMs[Asic];

    uint64_t TimeNS = ESSReadoutParser.Packet.Time.toNS(readout.TimeHigh, readout.TimeLow);
    int64_t TDCCorr = Calib.TDCCorr(readout.Channel, readout.TDC);
    XTRACE(DATA, DEB, "TimeNS raw %" PRIu64 ", correction %" PRIi64, TimeNS, TDCCorr);

    TimeNS += TDCCorr;
    XTRACE(DATA, DEB, "TimeNS corrected %" PRIu64, TimeNS);

    uint16_t ADC = Calib.ADCCorr(readout.Channel, readout.OTADC & 0x3FF);
    XTRACE(DATA, DEB, "ADC calibration from %u to %u", readout.OTADC & 0x3FF, ADC);

    // Now we add readouts with the calibrated time and adc to the x,y builders
    if (Geom.isXCoord(readout.VMM)) {
      XTRACE(DATA, DEB, "X: TimeNS %" PRIu64 ", Plane %u, Coord %u, Channel %u, ADC %u",
         TimeNS, PlaneX, Geom.xCoord(readout.VMM, readout.Channel), readout.Channel, ADC);
      builders[Hybrid].insert({TimeNS, Geom.xCoord(readout.VMM, readout.Channel),
                      ADC, PlaneX});

      ADCHist.bin_x(Hybrid * 64 + readout.Channel, ADC);
      //TDCHist.bin_x(Hybrid * 64 + readout.Channel, TDCCorr);

    } else { // implicit isYCoord
      XTRACE(DATA, DEB, "Y: TimeNS %" PRIu64 ", Plane %u, Coord %u, Channel %u, ADC %u",
         TimeNS, PlaneY, Geom.yCoord(Cassette, readout.VMM, readout.Channel), readout.Channel, ADC);
      builders[Hybrid].insert({TimeNS, Geom.yCoord(Cassette, readout.VMM, readout.Channel),
                      ADC, PlaneY});

      ADCHist.bin_y(Hybrid * 64 + readout.Channel, ADC);
      //TDCHist.bin_x(Hybrid * 64 + readout.Channel, TDCCorr);
    }
  }

  for (auto & builder : builders) {
    builder.flush(); // Do matching
  }
}


void FreiaInstrument::generateEvents(std::vector<Event> & Events) {
  ESSReadout::ESSTime & TimeRef = ESSReadoutParser.Packet.Time;

  for (const auto &e : Events) {
    if (e.empty()) {
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
    if (Conf.Parms.WireGapCheck) {
      if (e.ClusterB.hasGap(Conf.Parms.MaxGapWire)) {
        XTRACE(EVENT, DEB, "Event discarded due to wire gap");
        counters.EventsInvalidWireGap++;
        continue;
      }
    }

    if (Conf.Parms.StripGapCheck) {
        if (e.ClusterA.hasGap(Conf.Parms.MaxGapStrip)) {
        XTRACE(EVENT, DEB, "Event discarded due to strip gap");
        counters.EventsInvalidStripGap++;
        continue;
      }
    }

    counters.EventsMatchedClusters++;
    XTRACE(EVENT, DEB, "Event Valid\n %s", e.to_string({}, true).c_str());

    // Calculate TOF in ns
    uint64_t EventTime = e.time_start();

    XTRACE(EVENT, DEB, "EventTime %" PRIu64 ", TimeRef %" PRIu64,
           EventTime, TimeRef.TimeInNS);

    if (TimeRef.TimeInNS > EventTime) {
      XTRACE(EVENT, WAR, "Negative TOF!");
      counters.TimeErrors++;
      continue;
    }

    uint64_t TimeOfFlight = EventTime - TimeRef.TimeInNS;

    if (TimeOfFlight > Conf.Parms.MaxTOFNS) {
        XTRACE(DATA, WAR, "TOF larger than %u ns", Conf.Parms.MaxTOFNS);
      counters.TOFErrors++;
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

    XTRACE(EVENT, INF, "Time: %u TOF: %u, x %u, y %u, pixel %u",
           time, TimeOfFlight, x, y, PixelId);
    counters.TxBytes += Serializer->addEvent(TimeOfFlight, PixelId);
    counters.Events++;
  }
  Events.clear(); // else events will accumulate
}


void FreiaInstrument::processMonitorReadouts(void) {
  ESSReadout::ESSTime & TimeRef = ESSReadoutParser.Packet.Time;
  // All readouts are potentially now valid, negative TOF is not
  // possible, but rings and fens
  // could still be outside the configured range, also
  // illegal time intervals can be detected here
  assert(Serializer != nullptr);
  Serializer->pulseTime(ESSReadoutParser.Packet.Time.TimeInNS); /// \todo sometimes PrevPulseTime maybe?

  XTRACE(DATA, DEB, "processMonitorReadouts()");
  for (const auto & readout : VMMParser.Result) {

    if (DumpFile) {
      dumpReadoutToFile(readout);
    }

    XTRACE(DATA, DEB, "readout: RingId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.RingId, readout.FENId, readout.VMM, readout.Channel, readout.TimeLow);

    if (readout.RingId/2 != 11) {
      XTRACE(DATA, WAR, "Invalid ring %u for monitor readout",
             readout.RingId);
      counters.RingErrors++;
      continue;
    }

    if (readout.FENId != 1) {
      XTRACE(DATA, WAR, "Invalid FEN %d for monitor readout",
             readout.FENId);
      counters.FENErrors++;
      continue;
    }

    uint64_t TimeNS = ESSReadoutParser.Packet.Time.toNS(readout.TimeHigh, readout.TimeLow);
    XTRACE(DATA, DEB, "TimeRef PrevTime %" PRIi64 "", TimeRef.PrevTimeInNS);
    XTRACE(DATA, DEB, "TimeRef CurrTime %" PRIi64 "", TimeRef.TimeInNS);
    XTRACE(DATA, DEB, "Time of readout  %" PRIi64 "", TimeNS);

    uint64_t TimeOfFlight = 0;
    if (TimeRef.TimeInNS > TimeNS) {
      TimeOfFlight = TimeNS - TimeRef.PrevTimeInNS;
    } else {
      TimeOfFlight = TimeNS - TimeRef.TimeInNS;
    }

    if (TimeOfFlight > Conf.Parms.MaxTOFNS) {
      XTRACE(DATA, WAR, "TOF larger than %u ns", Conf.Parms.MaxTOFNS);
      counters.TOFErrors++;
      continue;
    }

    if (readout.BC != 0) {
       XTRACE(DATA, WAR, "BC (%u) must be zero", readout.BC);
       counters.MonitorErrors++;
       continue;
    }

    if (readout.OTADC != 0) {
       XTRACE(DATA, WAR, "OTADC (%u) must be zero", readout.OTADC);
       counters.MonitorErrors++;
       continue;
    }

    if (readout.GEO != 0) {
       XTRACE(DATA, WAR, "GEO (%u) must be zero", readout.GEO);
       counters.MonitorErrors++;
       continue;
    }

    if (readout.TDC != 0) {
       XTRACE(DATA, WAR, "TDC (%u) must be zero", readout.TDC);
       counters.MonitorErrors++;
       continue;
    }

    if (readout.VMM != 0) {
       XTRACE(DATA, WAR, "VMM (%u) must be zero", readout.VMM);
       counters.MonitorErrors++;
       continue;
    }

    if (readout.Channel != 0) {
       XTRACE(DATA, WAR, "Channel (%u) must be zero", readout.Channel);
       counters.MonitorErrors++;
       continue;
    }

    XTRACE(DATA, DEB, "TOF %" PRIu64 "", TimeOfFlight);
    counters.TxBytes += Serializer->addEvent(TimeOfFlight, 1);
    counters.MonitorCounts++;
  }
}


/// \todo move into readout/vmm3 instead as this will be common
void FreiaInstrument::dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data & Data) {
  VMM3::Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
  CurrentReadout.PrevPulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
  CurrentReadout.PrevPulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
  CurrentReadout.EventTimeHigh = Data.TimeHigh;
  CurrentReadout.EventTimeLow = Data.TimeLow;
  CurrentReadout.OutputQueue = ESSReadoutParser.Packet.HeaderPtr->OutputQueue;
  CurrentReadout.BC = Data.BC;
  CurrentReadout.OTADC = Data.OTADC;
  CurrentReadout.GEO = Data.GEO;
  CurrentReadout.TDC = Data.TDC;
  CurrentReadout.VMM = Data.VMM;
  CurrentReadout.Channel = Data.Channel;
  CurrentReadout.RingId = Data.RingId;
  CurrentReadout.FENId = Data.FENId;

  DumpFile->push(CurrentReadout);
}


} // namespace
