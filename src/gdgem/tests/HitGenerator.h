/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file      */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator for artificial Hits
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/Hit.h>
#include <cstdint>
#include <random>

namespace Gem {
  class HitGenerator {
  public:

    /// \brief configure absolute time and gap between events
    void setTimes(uint64_t TimeZero, uint32_t TimeGap, uint32_t DeltaT) {
      T0 = TimeZero;
      TGap = TimeGap;
      dT = DeltaT;
    }

    std::vector<Hit> & makeHit(uint8_t MaxReadouts,
         uint16_t X0, uint16_t Y0, float Theta, bool Shuffle);

    /// \brief Reconfigure absolute time and gap between events
    std::vector<Hit> & makeHits(uint32_t EventCount, uint8_t MaxReadouts, bool Shuffle);

    /// \brief debug and testing function
    void printHits();

  private:
    uint64_t T0{0}; // ns
    uint32_t TGap{100}; // ns between events
    uint32_t dT{1}; // ns between readouts
    std::vector<Hit> Hits;
    std::mt19937 RandGen; ///< mersenne twister random number generator
    const uint8_t PlaneX{0};
    const uint8_t PlaneY{1};
  };
} // namespace
