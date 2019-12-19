/** Copyright (C) 2019 European Spallation Source ERIC */


#include <gdgem/tests/HitGenerator.h>
#include <fmt/format.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

/// \todo Should not belong to gdgem but to common, reduction?
namespace Gem {

void HitGenerator::printHits() {
  for (auto & Hit : Hits) {
    fmt::print("{}, {}, {}, {}\n", Hit.time, Hit.plane, Hit.coordinate, Hit.weight);
  }
}

std::vector<Hit> & HitGenerator::makeHit(uint8_t MaxReadouts,
     uint16_t X0, uint16_t Y0, float Theta, bool Shuffle) {

  uint64_t Time = T0;
  std::vector<Hit> TmpHits;
  uint16_t OldX{65535};
  uint16_t OldY{65535};
  for (unsigned int RO = 0; RO < MaxReadouts; RO++) {
    uint16_t ChX = X0 + RO * 1.0 * cos(Theta);
    uint16_t ChY = Y0 + RO * 1.0 * sin(Theta);

    if ((OldX == ChX) and (OldY == ChY)) {
      continue;
    }

    Hit HitX{Time, ChX, 2345, PlaneX};
    TmpHits.push_back(HitX);
    Time += dT;
    OldX = ChX;

    Hit HitY{Time, ChY, 5432, PlaneY};
    TmpHits.push_back(HitY);
    Time += dT;
    OldY = ChY;
  }

  if (Shuffle) {
    std::shuffle(TmpHits.begin(), TmpHits.end(), RandGen);
  }

  for (auto & Hit : TmpHits) {
    Hits.push_back(Hit);
  }
  return Hits;
}

std::vector<Hit> & HitGenerator::makeHits(
    uint32_t EventCount, uint8_t MaxReadouts, bool Shuffle) {
  uint64_t Time = T0;
  std::vector<Hit> TmpHits;
  for (unsigned int Event = 0; Event < EventCount; Event++) {
    uint16_t Coord = Event;
    for (unsigned int RO = 0; RO < MaxReadouts; RO++) {
      uint16_t Weight = 2345;
      Hit HitX{Time, Coord, Weight, PlaneX};
      TmpHits.push_back(HitX);
      Coord++;
      Time += dT;
    }
    Time += TGap;
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
