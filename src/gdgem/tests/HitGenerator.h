/* Copyright (C) 2019, 2020 European Spallation Source, ERIC. See LICENSE file*/
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
    /// \param TimeZero start time of Hits
    /// \param TimeGap timegap between Events (groups of Hits)
    /// \param DeltaT timegap between Hits
    void setTimeParms(uint64_t TimeZero, uint32_t TimeGap, uint32_t DeltaT) {
      T0 = TimeZero;
      TGap = TimeGap;
      dT = DeltaT;
    }

    /// \brief make hits for a single plane
    /// \param Plane 0 for x, 1 for y
    /// \param MaxHits maximum number of hits generated (depends on angle)
    /// \param X0 x-coordinate for entry point of 'neutron'
    /// \param Y0 y-coordinate for entry point of 'neutron'
    /// \param Degrees angle of track in degrees
    /// \param Gaps number of gaps in generated Hits
    /// \param DeadTime minimum time (ns) between Hits for same channel
    /// \param Shuffle randomly shuffles the Hits
    /// \todo Should deadtime comparison be strictly less than or less than or equal?
    /// For now it is strictly less than. Hence (oldtime - time) == 1 is within DT
    /// and (OT - NT) == 2 is outside DT if DT == 2
    std::vector<Hit> & makeHitsForSinglePlane(uint8_t Plane, uint8_t MaxHits,
         uint16_t X0, uint16_t Y0, float Degrees, uint8_t Gaps, uint32_t DeadTime, bool Shuffle);

    /// \brief debug and testing function
    void printHits();

    ///
    std::vector<Hit> & getHits() { return Hits; }

  private:
    /// \brief Convert degrees to radians
    float D2R(float Degrees) {
      auto res = 2 * M_PI * Degrees / 360.0;
      return res;
    }

    uint64_t T0{0}; // ns
    uint32_t TGap{100}; // ns between events
    uint32_t dT{1}; // ns between Hits
    uint16_t CoordMin{0};
    uint16_t CoordMax{1279};

    std::vector<Hit> Hits;
    std::mt19937 RandGen; ///< mersenne twister random number generator
    const uint8_t PlaneX{0};
    const uint8_t PlaneY{1};
  };
} // namespace
