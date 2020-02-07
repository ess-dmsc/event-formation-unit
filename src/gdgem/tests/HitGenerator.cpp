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

void HitGenerator::printHits() {
  for (auto & Hit : Hits) {
    fmt::print("t {}, p {}, c {}, w {}\n", Hit.time, Hit.plane, Hit.coordinate, Hit.weight);
  }
}


std::vector<Hit> & HitGenerator::makeHitsForSinglePlane(uint8_t Plane, uint8_t MaxHits,
         uint16_t X0, uint16_t Y0, float Angle, uint8_t __attribute__((unused)) Gaps, uint32_t DeadTimeNs, bool Shuffle) {

  uint64_t Time = T0;
  uint32_t OldTime = Time;
  int16_t TmpCoord{0};
  uint16_t Coord{32767};
  uint16_t OldCoord{32767};
  uint16_t ADC{2345};
  std::vector<Hit> TmpHits;

  assert((Plane == PlaneX) or (Plane == PlaneY));

  for (unsigned int RO = 0; RO < MaxHits; RO++) {
    if (Plane == 0) {
      TmpCoord = X0 + (int16_t)(RO * 1.0 * cos(D2R(Angle)));
      //fmt::print("X0 {}, RO {}, Angle {}, cos(angle) {}, TmpCoord {}\n", X0, RO, Angle, cosa, TmpCoord);
    } else {
      TmpCoord = Y0 + (int16_t)(RO * 1.0 * sin(D2R(Angle)));
    }

    if ((TmpCoord > CoordMax) or (TmpCoord < CoordMin)) {
      //fmt::print("Coordinate {} out of bounds\n", Coord);
      continue;
    }
    Coord = (uint16_t)TmpCoord;
    //fmt::print("Candidate: plane {}, time {}, coord {}, oldtime {}, oldcoord {}\n",
    //           Plane, Time, Coord, OldTime, OldCoord);

    // Check coordinate and time conditions
    // Same coordinate - time gap is critical
    if (Coord == OldCoord) {
      //fmt::print("Same coordinate, oletime {}, Time {}\n", OldTime, Time);
      if ((OldTime != Time) and (Time - OldTime < DeadTimeNs) ) {
        // Not the first Hit but within deadtime
        //fmt::print("  dT {} shorter than deadtime - skipping\n", Time - OldTime);
        Time += dT;
        continue;
      }
    }

    Hit CurrHit{Time, Coord, ADC, Plane};
    TmpHits.push_back(CurrHit);
    OldTime = Time;
    Time += dT;
    OldCoord = Coord;
  }

  if (Shuffle) {
    std::shuffle(TmpHits.begin(), TmpHits.end(), RandGen);
  }

  for (auto & Hit : TmpHits) {
    Hits.push_back(Hit);
  }
  return Hits;
}


} // namespace
