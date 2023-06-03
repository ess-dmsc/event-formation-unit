// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Timepix3 processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <fmt/format.h>
#include <timepix3/Timepix3Instrument.h>
#include <math.h>     

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

/// \brief load configuration and calibration files, throw exceptions
/// if these have errors or are inconsistent
///
/// throws if number of pixels do not match, and if the (invalid) pixel
/// value 0 is mapped to a nonzero value
Timepix3Instrument::Timepix3Instrument(struct Counters &counters,
                                       BaseSettings &settings)
    : counters(counters), Settings(settings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  Timepix3Configuration = Config(Settings.ConfigFile);

  Geom = new Geometry();
  Geom->ESSGeom = new ESSGeometry(Timepix3Configuration.XResolution,
                                  Timepix3Configuration.YResolution, 1, 1);

  Clusterer = new HierarchicalClusterer(Timepix3Configuration.MaxTimeGapNS, Timepix3Configuration.MaxCoordinateGap);


  // if (not Settings.DumpFilePrefix.empty()) {
  //   if (boost::filesystem::path(Settings.DumpFilePrefix).has_extension()) {

  //     DumpFile =
  //         ReadoutFile::create(boost::filesystem::path(Settings.DumpFilePrefix)
  //                                 .replace_extension(""));
  //   } else {
  //     DumpFile =
  //         ReadoutFile::create(Settings.DumpFilePrefix + "_" + timeString());
  //   }
  // }
}

Timepix3Instrument::~Timepix3Instrument() {}

/// \brief helper function to calculate pixels from knowledge about
/// timepix3 panel, FENId and a single readout dataset
///
/// also applies the calibration
uint32_t Timepix3Instrument::calcPixel(DataParser::Timepix3PixelReadout &Data) {
  XTRACE(DATA, DEB, "Calculating pixel");

  uint32_t pixel = Geom->calcPixel(Data);
  XTRACE(DATA, DEB, "Calculated pixel to be %u", pixel);
  return pixel;
}

uint64_t
Timepix3Instrument::calcTimeOfFlight(DataParser::Timepix3PixelReadout &Data) {
  XTRACE(DATA, DEB, "Calculating TOF");
  XTRACE(DATA, DEB, "ToA: %u, FToA: %u, Spidr_time: %u", Data.ToA, Data.FToA,
         Data.SpidrTime);
  // uint64_t ToF = 25 * Data.ToA - 1.5625 * Data.FToA;
  uint64_t ToF =
      int(409600 * Data.SpidrTime + 25 * Data.ToA - 1.5625 * Data.FToA);
  XTRACE(DATA, DEB, "%u", ToF);
  return ToF;
}

// TODO, fix this
// void Timepix3Instrument::dumpReadoutToFile(DataParser::Timepix3PixelReadout
// &Data) {
//   Readout CurrentReadout;
//   CurrentReadout.PulseTimeHigh =
//   ESSReadoutParser.Packet.HeaderPtr->PulseHigh; CurrentReadout.PulseTimeLow =
//   ESSReadoutParser.Packet.HeaderPtr->PulseLow;
//   CurrentReadout.PrevPulseTimeHigh =
//       ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
//   CurrentReadout.PrevPulseTimeLow =
//       ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
//   CurrentReadout.EventTimeHigh = Data.TimeHigh;
//   CurrentReadout.EventTimeLow = Data.TimeLow;
//   CurrentReadout.DataSeqNum = Data.DataSeqNum;
//   CurrentReadout.OutputQueue =
//   ESSReadoutParser.Packet.HeaderPtr->OutputQueue; CurrentReadout.AmpA =
//   Data.AmpA; CurrentReadout.AmpB = Data.AmpB; CurrentReadout.AmpC =
//   Data.AmpC; CurrentReadout.AmpD = Data.AmpD; CurrentReadout.RingId =
//   Data.RingId; CurrentReadout.FENId = Data.FENId; CurrentReadout.TubeId =
//   Data.TubeId; DumpFile->push(CurrentReadout);
// }


void Timepix3Instrument::processReadouts() {

  // TODO - handle changing reference time mid-packet
  Serializer->setReferenceTime(Timepix3Parser.LastEVRTime);

  /// Traverse readouts, push back to AllHits
  for (auto &Data : Timepix3Parser.PixelResult) {
    bool ValidData = Geom->validateData(Data);
    if (not ValidData) {
      XTRACE(DATA, WAR, "Invalid Data, skipping readout");
      continue;
    }

    // if (DumpFile) {
    //   dumpReadoutToFile(Data);
    // }

    // Calculate TOF in ns
    uint16_t TimeOfFlight = calcTimeOfFlight(Data); 
    uint16_t X = Geom->calcX(Data);
    uint16_t Y = Geom->calcY(Data);
    uint16_t ToT = Data.ToT;

    XTRACE(DATA, DEB, "Parsed new hit, ToF: %u, X: %u, Y: %u, ToT: %u", TimeOfFlight, X, Y, ToT);
    AllHitsVector.push_back({TimeOfFlight, X, Y, ToT});

  }

  // sort hits by time of flight for clustering in time
  Clusterer->clusterNonConst(AllHitsVector);
  generateEvents();
  AllHitsVector.clear();
}

void Timepix3Instrument::generateEvents(){
  for (auto cluster:  Clusterer->clusters){
  
    uint64_t EventTime = cluster.timeStart();
    uint16_t x = cluster.xCoordCenter();
    uint16_t y = cluster.yCoordCenter();
    uint32_t PixelId = Geom->ESSGeom->pixel2D(x, y);
    if (PixelId == 0) {
      XTRACE(EVENT, WAR, "Bad pixel!: Time: %u, x %u, y %u, pixel %u",
              EventTime, x, y, PixelId);
      counters.PixelErrors++;
      continue;
    }
    XTRACE(EVENT, DEB, "New event, Time: %u, PixelId: %u", EventTime, PixelId);
    counters.TxBytes += Serializer->addEvent(EventTime, PixelId);
    counters.Events++;
  }
  Clusterer->clusters.clear();
}
} // namespace Timepix3
