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
         Data.spidr_time);
  // uint64_t ToF = 25 * Data.ToA - 1.5625 * Data.FToA;
  uint64_t ToF =
      int(409600 * Data.spidr_time + 25 * Data.ToA + 1.5625 * Data.FToA);
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

bool compareByTimeOfFlight(const TimepixHit& hit1, const TimepixHit& hit2) {
  return hit1.TimeOfFlight < hit2.TimeOfFlight;
}

void Timepix3Instrument::processReadouts() {

  /// vector of all hits in packet - so hits can be sorted before event formation
  std::vector<TimepixHit> all_hits;

  /// vector of hits per event - cleared after event pushed to serializer
  std::vector<TimepixHit> event_hits;

  /// Traverse readouts, push back to all_hits
  for (auto &Data : Timepix3Parser.PixelResult) {
    bool validData = Geom->validateData(Data);
    if (not validData) {
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

    all_hits.push_back({X, Y, TimeOfFlight, ToT});

  }

  // sort hits by time of flight for clustering in time
  std::sort(all_hits.begin(), all_hits.end(), compareByTimeOfFlight);

  // iterate over sorted hits, clustering into events
  for (TimepixHit hit:all_hits){
    uint64_t TimeOfFlight = hit.TimeOfFlight; 
    uint32_t X = hit.X;
    uint32_t Y = hit.Y;
    uint16_t ToT = hit.ToT;
    // std::cout << TimeOfFlight << std::endl;

    if ((not event_hits.empty()) and
        (TimeOfFlight > event_hits.back().TimeOfFlight + MaxTimeGapNS)) {
      XTRACE(DATA, DEB, "Time gap greater than %u, forming new event", MaxTimeGapNS);
      if (event_hits.size() < MinEventSizeHits) {
        event_hits.clear();
        continue;
      }
      uint64_t sum_X = 0;
      uint64_t sum_Y = 0;
      uint64_t sum_ToT = 0;
      for (TimepixHit h : event_hits) {
        sum_X += h.X;
        sum_Y += h.Y;
        sum_ToT += h.ToT;
      }
      if (sum_ToT < MinimumToTSum) {
        event_hits.clear();
        continue;
      }
      uint32_t X = sum_X / event_hits.size();
      uint32_t Y = sum_Y / event_hits.size();
      uint32_t PixelId = Geom->ESSGeom->pixel2D(X, Y);
      uint64_t ToF = event_hits.front().TimeOfFlight;
      counters.TxBytes += Serializer->addEvent(ToF, PixelId);
      counters.Events++;
      // std::cout << X << " " << Y << std::endl;
      XTRACE(DATA, DEB, "Added event to serializer, hits: %u, ToF: %u, PixelId: %u", event_hits.size(), ToF, PixelId);
      event_hits.clear();
    }

    event_hits.push_back({X, Y, TimeOfFlight, ToT});
    XTRACE(DATA, DEB, "Added hit to event, event now contains %u hits", event_hits.size());

  } // for()
}

} // namespace Timepix3
