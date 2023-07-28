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
Timepix3Instrument::Timepix3Instrument(struct Counters &counters,
                                       BaseSettings &settings)
    : counters(counters), Settings(settings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  Timepix3Configuration = Config(Settings.ConfigFile);

  
  Geom = new Geometry();
  Geom->setXResolution(Timepix3Configuration.XResolution);
  Geom->setYResolution(Timepix3Configuration.YResolution);
  Geom->ESSGeom = new ESSGeometry(Timepix3Configuration.XResolution,
                                  Timepix3Configuration.YResolution, 1, 1);

  Clusterer = new HierarchicalClusterer(Timepix3Configuration.MaxTimeGapNS, Timepix3Configuration.MaxCoordinateGap);

}

Timepix3Instrument::~Timepix3Instrument() {}

/// \brief helper function to calculate pixels from timepix3 data
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
  // this formula is based on the information in the timepix3 manual supplied with the camera
  uint64_t ToF =
      int(409600 * Data.SpidrTime + 25 * Data.ToA - 1.5625 * Data.FToA);
  XTRACE(DATA, DEB, "ToF: %u", ToF);
  return ToF;
}


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

    // Calculate TOF in ns
    uint16_t TimeOfFlight = calcTimeOfFlight(Data); 
    uint16_t X = Geom->calcX(Data);
    uint16_t Y = Geom->calcY(Data);
    uint16_t ToT = Data.ToT;

    XTRACE(DATA, DEB, "Parsed new hit, ToF: %u, X: %u, Y: %u, ToT: %u", TimeOfFlight, X, Y, ToT);
    AllHitsVector.push_back({TimeOfFlight, X, Y, ToT});

  }

  // sort hits by time of flight for clustering in time
  sort_chronologically(std::move(AllHitsVector)); 
  Clusterer->cluster(AllHitsVector);
  generateEvents();
  AllHitsVector.clear();
}

void Timepix3Instrument::generateEvents(){
  for (auto cluster:  Clusterer->clusters){
    // other options for time are timeEnd, timeCenter, etc. we picked timeStart for this type of 
    //detector, it is the time the first photon in the cluster hit the detector.
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
