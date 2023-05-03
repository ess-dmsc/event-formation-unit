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

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

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
      int(409600 * Data.SpidrTime + 25 * Data.ToA + 1.5625 * Data.FToA);
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

struct TimepixHit {
  uint32_t X;
  uint32_t Y;
  uint64_t TimeOfFlight;
  uint16_t ToT;
};

bool compareByTimeOfFlight(const TimepixHit& Hit1, const TimepixHit& Hit2) {
  return Hit1.TimeOfFlight < Hit2.TimeOfFlight;
}

std::vector<std::vector<TimepixHit>> clusterByCoordinate(std::vector<TimepixHit> EventHits, double distanceThreshold) {
    std::vector<std::vector<TimepixHit>> clusters;
    std::set<int> visited; // keep track of visited points
    XTRACE(DATA, DEB, "%u events in time window", EventHits.size());
    for (uint i = 0; i < EventHits.size(); i++) {
        if (visited.count(i) > 0) {
            continue; // skip points that have already been visited
        }
        XTRACE(DATA, DEB, "Starting new cluster");
        std::vector<TimepixHit> cluster; // initialize a new cluster
        cluster.push_back(EventHits[i]);
        visited.insert(i);
        for (uint j = i+1; j < EventHits.size(); j++) {
            if (visited.count(j) > 0) {
                continue; // skip points that have already been visited
            }
            double distance = sqrt(pow((double)EventHits[i].X - (double)EventHits[j].X, 2) + pow((double)EventHits[i].Y - (double)EventHits[j].Y, 2));
            XTRACE(DATA, DEB, "Determined distance between points is %f, threshold is %f", distance, distanceThreshold);
            XTRACE(DATA, DEB, "X1 = %u, X2 = %u, Y1 = %u, Y2 = %u", EventHits[i].X, EventHits[j].X, EventHits[i].Y, EventHits[j].Y);
            if (distance < distanceThreshold) {
                XTRACE(DATA, DEB, "Adding to existing cluster");
                cluster.push_back(EventHits[j]); // add point to current cluster
                visited.insert(j);
            }
            else{
              XTRACE(DATA, DEB, "Too far apart, not including in this cluster");
            }
        }
        
        clusters.push_back(cluster); // add completed cluster to list of clusters
    }
    
    return clusters;
}



void Timepix3Instrument::processReadouts() {

  /// vector of all hits in packet - so hits can be sorted before event formation
  std::vector<TimepixHit> AllHits;

  /// vector of hits per event - cleared after event pushed to serializer
  std::vector<TimepixHit> ClusteredInTimeEventHits;

  std::vector<std::vector<TimepixHit>> ClusteredInSpaceEventHits;

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
    uint64_t TimeOfFlight = calcTimeOfFlight(Data); 
    uint32_t X = Geom->calcX(Data);
    uint32_t Y = Geom->calcY(Data);
    uint16_t ToT = Data.ToT;

    AllHits.push_back({X, Y, TimeOfFlight, ToT});

  }

  // sort hits by time of flight for clustering in time
  std::sort(AllHits.begin(), AllHits.end(), compareByTimeOfFlight);

  // iterate over sorted hits, clustering into events
  for (TimepixHit Hit:AllHits){
    uint64_t TimeOfFlight = Hit.TimeOfFlight; 
    uint32_t X = Hit.X;
    uint32_t Y = Hit.Y;
    uint16_t ToT = Hit.ToT;
    // std::cout << TimeOfFlight << std::endl;

    if ((not ClusteredInTimeEventHits.empty()) and
        (TimeOfFlight > ClusteredInTimeEventHits.back().TimeOfFlight + MaxTimeGapNS)) {
      XTRACE(DATA, DEB, "Time gap greater than %u, forming new events", MaxTimeGapNS);
      // EventHits contains hits in given time window, now need clustering by coordinate
      ClusteredInSpaceEventHits = clusterByCoordinate(ClusteredInTimeEventHits, 10.0);
      XTRACE(DATA, DEB, "Split time window of %u photon events into clusters by coordinate, made %u clusters.", ClusteredInTimeEventHits.size(), ClusteredInSpaceEventHits.size() );
      ClusteredInTimeEventHits.clear();

      for(std::vector<TimepixHit> EventHits : ClusteredInSpaceEventHits){
        if (EventHits.size() < MinEventSizeHits) {
          XTRACE(DATA, DEB, "Not enough events in cluster, got %u, minimum %u", EventHits.size(), MinEventSizeHits);
          continue;
        }
      
        uint64_t SumX = 0;
        uint64_t SumY = 0;
        uint64_t SumToT = 0;
        for (TimepixHit EventHit : EventHits) {
          SumX += EventHit.X;
          SumY += EventHit.Y;
          SumToT += EventHit.ToT;
        }
        if (SumToT < MinimumToTSum) {
          XTRACE(DATA, DEB, "Not high enough ToT, got %u, minimum %u", SumToT, MinimumToTSum);
          continue;
        }
        uint32_t X = SumX / EventHits.size();
        uint32_t Y = SumY / EventHits.size();
        uint32_t PixelId = Geom->ESSGeom->pixel2D(X, Y);
        uint64_t ToF = EventHits.front().TimeOfFlight;
        counters.TxBytes += Serializer->addEvent(ToF, PixelId);
        counters.Events++;
        std::cout << X << " " << Y << " "<< ToF << std::endl;
        XTRACE(DATA, DEB, "Added event to serializer, hits: %u, ToF: %u, PixelId: %u", EventHits.size(), ToF, PixelId);
      }
      ClusteredInSpaceEventHits.clear();
    }

    ClusteredInTimeEventHits.push_back({X, Y, TimeOfFlight, ToT});
    XTRACE(DATA, DEB, "Added hit to time window, window now contains %u hits", ClusteredInTimeEventHits.size());

  } // for()
}

} // namespace Timepix3
