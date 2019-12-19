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

std::vector<Hit> & HitGenerator::makeHit(uint8_t MaxHits,
     uint16_t X0, uint16_t Y0, float Theta, bool Shuffle) {

  uint64_t Time = T0;
  std::vector<Hit> TmpHits;
  uint16_t OldX{65535};
  uint16_t OldY{65535};
  for (unsigned int RO = 0; RO < MaxHits; RO++) {
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

} // namespace
