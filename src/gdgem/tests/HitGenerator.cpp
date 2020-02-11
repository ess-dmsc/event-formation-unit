/** Copyright (C) 2019 European Spallation Source ERIC */


#include <gdgem/tests/HitGenerator.h>
#include <fmt/format.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cassert>

/// \todo Should not belong to gdgem but to common, reduction?
namespace Gem {

//
void HitGenerator::printHits() {
  for (auto & Hit : Hits) {
    fmt::print("t {}, p {}, c {}, w {}\n", Hit.time, Hit.plane, Hit.coordinate, Hit.weight);
  }
}

//
std::vector<NeutronEvent> & HitGenerator::randomEvents(int NEvents, int MinCoord, int MaxCoord) {
  auto Time = T0;
  std::uniform_int_distribution<int> coords(MinCoord, MaxCoord);
  for (int i = 0; i < NEvents; i++) {
    auto XPos = coords(RandGen);
    auto YPos = coords(RandGen);
    //fmt::print("Event: ({}, {}), t={}\n", XPos, YPos, Time);
    Events.push_back({XPos, YPos, Time});
    Time += TGap;
  }
  return Events;
}

//
std::vector<Hit> & HitGenerator::randomHits(int MaxHits, int Gaps, int DeadTime, bool Shuffle) {
  std::uniform_int_distribution<float> angle(0.0, 360.0);
  for (auto & Event : Events) {
    auto Degrees = angle(RandGen);
    //fmt::print("({},{}) @ {}\n", Event.XPos, Event.YPos, Degrees);
    makeHitsForSinglePlane(0, MaxHits, Event.XPos, Event.YPos, Degrees, Gaps, DeadTime, Shuffle);
    makeHitsForSinglePlane(1, MaxHits, Event.XPos, Event.YPos, Degrees, Gaps, DeadTime, Shuffle);
    advanceTime(TGap);
  }
  return Hits;
}

//
std::vector<Hit> & HitGenerator::makeHitsForSinglePlane(int Plane, int MaxHits,
         float X0, float Y0, float Angle, int Gaps, int DeadTimeNs, bool Shuffle) {
  int64_t Time = T0;
  int32_t OldTime = Time;
  float TmpCoord{0};
  int Coord{32767}, OldCoord{32767};
  uint16_t ADC{2345};
  std::vector<Hit> TmpHits;

  assert((Plane == PlaneX) or (Plane == PlaneY));

  for (int hit = 0; hit < MaxHits; hit++) {
    if (Plane == PlaneX) {
      TmpCoord = X0 + hit * 1.0 * cos(D2R(Angle));
      //fmt::print("X0 {}, RO {}, Angle {}, cos(angle) {}, TmpCoord {}\n", X0, hit, Angle, cosa, TmpCoord);
    } else {
      TmpCoord = Y0 + hit * 1.0 * sin(D2R(Angle));
    }

    Coord = (int)TmpCoord;
    if ((Coord > CoordMax) or (Coord < CoordMin)) {
      continue;
    }

    // Same coordinate - time gap is critical
    if (Coord == OldCoord) {
      //fmt::print("Same coordinate, OldTime {}, Time {}\n", OldTime, Time);
      if ((OldTime != Time) and (Time - OldTime < DeadTimeNs) ) {
        // Not the first Hit but within deadtime
        //fmt::print("  dT {} shorter than deadtime - skipping\n", Time - OldTime);
        Time += DeltaT;
        continue;
      }
    }

    Hit CurrHit{(uint64_t)Time, (uint16_t)Coord, ADC, (uint8_t)Plane};
    TmpHits.push_back(CurrHit);
    OldTime = Time;
    Time += DeltaT;
    OldCoord = Coord;
  }

  // Handle gaps
  TmpHits = makeGaps(TmpHits, Gaps);

  if (Shuffle) {
    std::shuffle(TmpHits.begin(), TmpHits.end(), RandGen);
  }

  for (auto & Hit : TmpHits) {
    Hits.push_back(Hit);
  }
  return Hits;
}

//
std::vector<Hit> & HitGenerator::makeGaps(std::vector<Hit> & Hits, uint8_t Gaps) {
  if (Gaps == 0) {
    return Hits;
  }
  std::vector<Hit> GapHits;
  if (Gaps > Hits.size() - 2) {
    //fmt::print("Gaps requestes {}, available {}\n", Gaps, TmpHits.size() - 2);
    Hits.clear();
  }
  for (unsigned int i = 0; i < Hits.size(); i ++) {
    if ((i == 0) or (i > Gaps)) {
      GapHits.push_back(Hits[i]);
    }
  }
  Hits = GapHits;
  return Hits;
}

} // namespace
