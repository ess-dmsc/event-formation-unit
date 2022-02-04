// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPECInstrument is responsible for readout validation and event
/// formation
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/Readout.h>
#include <common/time/TimeString.h>
#include <cspec/CSPECInstrument.h>
#include <cspec/geometry/CSPECGeometry.h>
#include <math.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Cspec {

/// \brief load configuration and calibration files
CSPECInstrument::CSPECInstrument(struct Counters &counters,
                                 // BaseSettings & EFUSettings,
                                 CSPECSettings &moduleSettings,
                                 EV42Serializer *serializer)
    : counters(counters), ModuleSettings(moduleSettings),
      Serializer(serializer) {
  if (!ModuleSettings.FilePrefix.empty()) {
    std::string DumpFileName =
        ModuleSettings.FilePrefix + "cspec_" + timeString();
    XTRACE(INIT, ALW, "Creating HDF5 dumpfile: %s", DumpFileName.c_str());
    DumpFile = VMM3::ReadoutFile::create(DumpFileName);
  }

  loadConfigAndCalib();

  essgeom =
      ESSGeometry(Conf.Parms.SizeX, Conf.Parms.SizeY, Conf.Parms.SizeZ, 1);

  // We can now use the settings in Conf.Parms
  if (Conf.Parms.InstrumentGeometry == "CSPEC") {
    GeometryInstance = &CSPECGeometryInstance;
  } else {
    throw std::runtime_error("Invalid InstrumentGeometry in config file");
  }

  XTRACE(INIT, ALW, "Set EventBuilder timebox to %u ns", Conf.Parms.TimeBoxNs);
  for (auto &builder : builders) {
    builder.setTimeBox(Conf.Parms.TimeBoxNs); // Time boxing
  }

  ESSReadoutParser.setMaxPulseTimeDiff(Conf.Parms.MaxPulseTimeNS);

  // Reinit histogram size (was set to 1 in class definition)
  // ADC is 10 bit 2^10 = 1024
  // Each plane (x,y) has a maximum of NumCassettes * 64 channels
  // eventhough there are only 32 wires so some bins will be empty
  // Hists will automatically allocate space for both x and y planes
  // uint32_t MaxADC = 1024;
  // uint32_t MaxChannels =
  //   Conf.NumHybrids * std::max(GeometryBase::NumWires,
  //   GeometryBase::NumStrips);
  // ADCHist = Hists(MaxChannels, MaxADC);
}

void CSPECInstrument::loadConfigAndCalib() {
  XTRACE(INIT, ALW, "Loading configuration file %s",
         ModuleSettings.ConfigFile.c_str());
  Conf = Config("CSPEC", ModuleSettings.ConfigFile);
  Conf.loadAndApply();

  // XTRACE(INIT, ALW, "Creating vector of %d builders (one per hybrid)",
  //        Conf.getNumHybrids());
  builders =
      std::vector<EventBuilder2D>((Conf.MaxRing + 1) * (Conf.MaxFEN + 1));

  // if (ModuleSettings.CalibFile != "") {
  //   XTRACE(INIT, ALW, "Loading and applying calibration file");
  //   ESSReadout::CalibFile Calibration("Freia", Hybrids);
  //   Calibration.load(ModuleSettings.CalibFile);
  // }
}

void CSPECInstrument::processReadouts(void) {
  // All readouts are potentially now valid, but rings and fens
  // could still be outside the configured range, also
  // illegal time intervals can be detected here
  assert(Serializer != nullptr);
  Serializer->pulseTime(ESSReadoutParser.Packet.Time
                            .TimeInNS); /// \todo sometimes PrevPulseTime maybe?

  XTRACE(DATA, DEB, "processReadouts()");
  for (const auto &readout : VMMParser.Result) {
    if (DumpFile) {
      dumpReadoutToFile(readout);
    }

    if (readout.RingId > Conf.MaxRing) {
      XTRACE(DATA, ERR, "Invalid Ring ID: %u, Max Ring ID: %u", readout.RingId,
             Conf.MaxRing);
      counters.RingErrors++;
      continue;
    }

    if (readout.FENId > Conf.MaxFEN) {
      XTRACE(DATA, ERR, "Invalid FEN ID: %u, Max FEN ID: %u", readout.FENId,
             Conf.MaxFEN);
      counters.FENErrors++;
      continue;
    }

    XTRACE(DATA, DEB,
           "readout: Phys RingId %d, FENId %d, VMM %d, Channel %d, TimeLow %d",
           readout.RingId, readout.FENId, readout.VMM, readout.Channel,
           readout.TimeLow);

    uint8_t HybridId = readout.VMM >> 1;
    if (!Conf.getHybrid(readout.RingId, readout.FENId, HybridId).Initialised) {
      XTRACE(DATA, WAR,
             "Hybrid for Ring %d, FEN %d, VMM %d not defined in config file",
             readout.RingId, readout.FENId, HybridId);
      counters.HybridErrors++;
      continue;
    }

    ESSReadout::Hybrid &Hybrid = Conf.getHybrid(readout.RingId, readout.FENId, HybridId); 
    // Convert from physical rings to logical rings
    // uint8_t Ring = readout.RingId/2;
    uint8_t AsicId = readout.VMM & 0x1;
    uint16_t XOffset = Hybrid.XOffset;
    uint16_t YOffset = Hybrid.YOffset;
    bool Rotated = Conf.Rotated[readout.RingId][readout.FENId][HybridId];
    bool Short = Conf.Short[readout.RingId][readout.FENId][HybridId];
    uint16_t MinADC = Conf.MinADC[readout.RingId][readout.FENId][HybridId];

    

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

    //   // Now we add readouts with the calibrated time and adc to the x,y
    //   builders
    // x and z coord is a combination of the X and Z coordinates that provides a
    // unique wire identifier Adjacency of wires isn't needed as wires are well
    // insulated and events don't span multiples of them
    if (GeometryInstance->isWire(HybridId)) {
      XTRACE(DATA, DEB, "Is wire, calculating x and z coordinate");
      uint16_t xAndzCoord = GeometryInstance->xAndzCoord(
          readout.FENId, HybridId, AsicId, readout.Channel, XOffset, Rotated);

      if (xAndzCoord == GeometryInstance->InvalidCoord) { // 65535 is invalid xandzCoordinate
        XTRACE(DATA, ERR, "Invalid X and Z Coord");
        counters.MappingErrors++;
        continue;
      }

      XTRACE(DATA, DEB, "XandZ: Coord %u, Channel %u", xAndzCoord,
             readout.Channel);
      builders[readout.RingId * Conf.MaxFEN + readout.FENId].insert(
          {TimeNS, xAndzCoord, ADC, 0});

      //     uint32_t GlobalXChannel = Hybrid * GeometryBase::NumStrips +
      //     readout.Channel; ADCHist.bin_x(GlobalXChannel, ADC);

    } else { // implicit isYCoord
      XTRACE(DATA, DEB, "Is grid, calculating y coordinate");
      uint16_t yCoord = GeometryInstance->yCoord(
          HybridId, AsicId, readout.Channel, YOffset, Rotated, Short);

      if (yCoord == GeometryInstance->InvalidCoord) { // invalid coordinate is 65535
        XTRACE(DATA, ERR, "Invalid Y Coord");
        counters.MappingErrors++;
        continue;
      }

      XTRACE(DATA, DEB, "Y: Coord %u, Channel %u", yCoord, readout.Channel);
      builders[readout.RingId * Conf.MaxFEN + readout.FENId].insert(
          {TimeNS, yCoord, ADC, 1});

      // uint32_t GlobalYChannel = Hybrid * GeometryBase::NumWires +
      // readout.Channel; ADCHist.bin_y(GlobalYChannel, ADC);
    }
  }

  for (auto &builder : builders) {
    builder.flush(); // Do matching
  }
}

void CSPECInstrument::generateEvents(std::vector<Event> &Events) {
  ESSReadout::ESSTime &TimeRef = ESSReadoutParser.Packet.Time;

  for (const auto &e : Events) {
    if (e.empty()) {
      continue;
    }

    if (!e.both_planes()) {
      XTRACE(EVENT, DEB, "Event has no coincidence");
      counters.ClustersNoCoincidence++;
      if (not e.ClusterB.empty()) {
        counters.ClustersMatchedGridOnly++;
      }

      if (not e.ClusterA.empty()) {
        counters.ClustersMatchedWireOnly++;
      }
      continue;
    }

    if (Conf.Parms.MaxGridsSpan < e.ClusterB.coord_span()) {
      XTRACE(EVENT, DEB, "Event spans too many grids, %u",
             e.ClusterA.coord_span());
      counters.ClustersTooLargeGridSpan++;
      continue;
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

    if (TimeOfFlight > Conf.Parms.MaxTOFNS) {
      XTRACE(DATA, WAR, "TOF larger than %u ns", Conf.Parms.MaxTOFNS);
      counters.TOFErrors++;
      continue;
    }

    // calculate local x and y using center of mass
    uint16_t xandz =
        static_cast<uint16_t>(std::round(e.ClusterA.coord_center()));
    uint16_t y = static_cast<uint16_t>(std::round(e.ClusterB.coord_center()));
    uint16_t x = floor(xandz / 16);
    uint16_t z = xandz % 16;
    auto PixelId = essgeom.pixel3D(x, y, z);

    if (PixelId == 0) {
      XTRACE(EVENT, WAR,
             "Bad pixel!: Time: %u TOF: %u, x %u, y %u, z %u, pixel %u", time,
             TimeOfFlight, x, y, z, PixelId);
      counters.PixelErrors++;
      continue;
    }

    XTRACE(EVENT, INF, "Time: %u TOF: %u, x %u, y %u, z %u, pixel %u", time,
           TimeOfFlight, x, y, z, PixelId);
    counters.TxBytes += Serializer->addEvent(TimeOfFlight, PixelId);
    counters.Events++;
  }
  Events.clear(); // else events will accumulate
}

/// \todo move into readout/vmm3 instead as this will be common
void CSPECInstrument::dumpReadoutToFile(
    const ESSReadout::VMM3Parser::VMM3Data &Data) {
  VMM3::Readout CurrentReadout;
  CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
  CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
  CurrentReadout.PrevPulseTimeHigh =
      ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
  CurrentReadout.PrevPulseTimeLow =
      ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
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

} // namespace Cspec
