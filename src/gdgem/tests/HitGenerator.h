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
  /// \param TimeZero Hits start at this time (ns)
  /// \param TimeGap Minimum time gap between events
  /// \param DeltaT Minimum timegap between Hits
  void setTimes(uint64_t TimeZero, uint32_t TimeGap, uint32_t DeltaT) {
    T0 = TimeZero;
    TGap = TimeGap;
    dT = DeltaT;
  }

  /// \brief Make Hit data corresponding to one event.
  /// \param MaxHits the maximum number of Hits generated
  /// \param X0 coordinate offset for planeX
  /// \param Y0 coordinate offset for planeY
  /// \param Theta angle (degree) of Hit 'track'
  /// \param Shuffle scrambles the Hits
  std::vector<Hit> &makeHit(uint8_t MaxHits, uint16_t X0, uint16_t Y0,
                            float Theta, bool Shuffle);

  /// \brief debug and testing function
  void printHits();

  std::vector<Hit> Hits;

private:
  uint64_t T0{0};       // ns
  uint32_t TGap{100};   // ns between events
  uint32_t dT{1};       // ns between hits
  std::mt19937 RandGen; ///< mersenne twister random number generator
  const uint8_t PlaneX{0};
  const uint8_t PlaneY{1};
};
} // namespace Gem
