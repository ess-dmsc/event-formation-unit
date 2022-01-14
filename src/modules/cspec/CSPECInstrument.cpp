// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPECInstrument is responsible for readout validation and event
/// formation
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <cspec/CSPECInstrument.h>
#include <cspec/geometry/CSPECGeometry.h>
#include <common/readout/vmm3/CalibFile.h>
#include <common/readout/vmm3/Readout.h>
#include <assert.h>
#include <math.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Cspec {

/// \brief load configuration and calibration files
CSPECInstrument::CSPECInstrument(struct Counters & counters,
  //BaseSettings & EFUSettings,
  CSPECSettings &moduleSettings,
  EV42Serializer * serializer)
    : counters(counters)
    , ModuleSettings(moduleSettings)
    , Serializer(serializer) {

  if (!ModuleSettings.FilePrefix.empty()) {
    std::string DumpFileName = ModuleSettings.FilePrefix + "cspec_" + timeString();
    XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
    DumpFile = VMM3::ReadoutFile::create(DumpFileName);
  }

  loadConfigAndCalib();

  // We can now use the settings in Conf.Parms
  if (Conf.Parms.InstrumentGeometry == "CSPEC"){
    Cspec::CSPECGeometry cspecgeometry = Cspec::CSPECGeometry();
    geometry = &cspecgeometry;
  }
  else {
    throw std::runtime_error("Invalid InstrumentGeometry in config file");
  }

  // XTRACE(INIT, ALW, "Set EventBuilder timebox to %u ns", Conf.Parms.TimeBoxNs);
  // for (auto & builder : builders) {
  //   builder.setTimeBox(Conf.Parms.TimeBoxNs); // Time boxing
  // }

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.Parms.MaxPulseTimeNS);

  // Reinit histogram size (was set to 1 in class definition)
  // ADC is 10 bit 2^10 = 1024
  // Each plane (x,y) has a maximum of NumCassettes * 64 channels
  // eventhough there are only 32 wires so some bins will be empty
  // Hists will automatically allocate space for both x and y planes
  // uint32_t MaxADC = 1024;
  // uint32_t MaxChannels =
  //   Conf.NumHybrids * std::max(GeometryBase::NumWires, GeometryBase::NumStrips);
  // ADCHist = Hists(MaxChannels, MaxADC);
}

void CSPECInstrument::loadConfigAndCalib() { }

void CSPECInstrument::processReadouts(void) {
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

    // counters.RingRx[readout.RingId]++;

    // Convert from physical rings to logical rings
    // uint8_t Ring = readout.RingId/2;
    uint8_t AsicId = readout.VMM & 0x1;
    uint8_t HybridId = floor(readout.VMM/2);

    if (!Conf.getHybrid(readout.RingId, readout.FENId, HybridId).Initialised) {
      XTRACE(DATA, WAR, "Hybrid for Ring %d, FEN %d, VMM %d not defined in config file",
              readout.RingId, readout.FENId, readout.VMM);
      counters.HybridErrors++;
      continue;
    }


  //   uint8_t Cassette = Hybrid + 1;
  //   VMM3Calibration & Calib = Hybrids[Hybrid].VMMs[Asic];

  //   uint64_t TimeNS = ESSReadoutParser.Packet.Time.toNS(readout.TimeHigh, readout.TimeLow);
  //   int64_t TDCCorr = Calib.TDCCorr(readout.Channel, readout.TDC);
  //   XTRACE(DATA, DEB, "TimeNS raw %" PRIu64 ", correction %" PRIi64, TimeNS, TDCCorr);

  //   TimeNS += TDCCorr;
  //   XTRACE(DATA, DEB, "TimeNS corrected %" PRIu64, TimeNS);

  //   // Only 10 bits of the 16-bit OTADC field is used hence the 0x3ff mask below
  //   uint16_t ADC = Calib.ADCCorr(readout.Channel, readout.OTADC & 0x3FF);

  //   XTRACE(DATA, DEB, "ADC calibration from %u to %u", readout.OTADC & 0x3FF, ADC);

  //   // If the corrected ADC reaches maximum value we count the occurance but
  //   // use the new value anyway
  //   if (ADC >= 1023) {
  //     counters.MaxADC++;
  //   }

  //   // Now we add readouts with the calibrated time and adc to the x,y builders
    if (geometry->isWire(HybridId)) {
      std::pair<uint8_t, uint8_t> xAndzCoord = geometry->xAndzCoord(readout.RingId, readout.FENId, HybridId, AsicId, readout.Channel);
      XTRACE(DATA, DEB, "X: Coord %u, Channel %u",
         xAndzCoord, readout.Channel) ;
      // builders[HybridId].insert({TimeNS, Geom.xCoord(readout.VMM, readout.Channel),
      //                 ADC, PlaneX});

  //     uint32_t GlobalXChannel = Hybrid * GeometryBase::NumStrips + readout.Channel;
  //     ADCHist.bin_x(GlobalXChannel, ADC);

    } else { // implicit isYCoord
      uint8_t yCoord = geometry->yCoord(HybridId, AsicId, readout.Channel);
      XTRACE(DATA, DEB, "Y: Coord %u, Channel %u",
         yCoord, readout.Channel) ;
      // builders[Hybrid].insert({TimeNS, Geom.yCoord(Cassette, readout.VMM, readout.Channel),
      //                 ADC, PlaneY});

      // uint32_t GlobalYChannel = Hybrid * GeometryBase::NumWires + readout.Channel;
      // ADCHist.bin_y(GlobalYChannel, ADC);
    }
  // }

  // for (auto & builder : builders) {
  //   builder.flush(); // Do matching
  }
}


void CSPECInstrument::generateEvents(std::vector<Event> & Events) {
  ESSReadout::ESSTime & TimeRef = ESSReadoutParser.Packet.Time;

  for (const auto &e : Events) {
    if (e.empty()) {
      continue;
    }

    if (!e.both_planes()) {
      XTRACE(EVENT, DEB, "Event has no coincidence");
      counters.EventsNoCoincidence++;
      continue;
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

    // if (TimeOfFlight > Conf.Parms.MaxTOFNS) {
    //     XTRACE(DATA, WAR, "TOF larger than %u ns", Conf.Parms.MaxTOFNS);
    //   counters.TOFErrors++;
    //   continue;
    // }

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


/// \todo move into readout/vmm3 instead as this will be common
void CSPECInstrument::dumpReadoutToFile(const ESSReadout::VMM3Parser::VMM3Data & Data) {
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
