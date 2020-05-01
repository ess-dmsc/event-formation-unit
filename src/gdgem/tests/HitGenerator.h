/* Copyright (C) 2019, 2020 European Spallation Source, ERIC. See LICENSE file*/
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator for artificial Events and Hits
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/Hit.h>
#include <cstdint>
#include <random>

namespace Gem {

  /// \todo replace by Martins Event primitive even if it contains stuff we
  /// don't care about.
  class NeutronEvent {
  public:
    int XPos{0}, YPos{0};
    uint64_t TimeNs;
  };

  class HitGenerator {
  public:
    /// \brief advance the internal time variable T0 by some amount
    /// \param TimeGapNS the amount (ns) to advance time.
    void advanceTime(uint32_t TimeGapNS) { T0 += TimeGapNS; }

    /// \brief configure absolute time and gap between events
    /// \param TimeZero start time of Hits
    /// \param TimeGap timegap between Events (groups of Hits)
    /// \param HitInc timegap between Hits
    void setTimeParms(uint64_t TimeZero, uint32_t TimeGap, uint32_t HitInc) {
      T0 = TimeZero;
      TGap = TimeGap;
      DeltaT = HitInc;
    }

    /// \brief generate hits for a number of random neutron events within
    /// a coordinate region. This method DOES NOT advance internal time.
    /// \param NEvents number of events (x,y,t)
    /// \param MinCoord minimum coordinate value
    /// \param MaxCoord maxumum coordinate value
    std::vector<NeutronEvent> & randomEvents(int NumEvents, int MinCoord, int MaxCoord);

    /// \brief generate Hits with random angles based on positions in Events
    /// The parameters MaxHits, Gaps, DeadTime and Shuffle are the same as the ones
    /// used in makeHitsForSinglePlane() below. This method DO ADVANCE internal time.
    std::vector<Hit> & randomHits(int MaxHits, int Gaps, int DeadTimeNs, bool Shuffle);

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
    std::vector<Hit> & makeHitsForSinglePlane(int Plane, int MaxHits,
         float X0, float Y0, float Degrees, int Gaps, int DeadTime, bool Shuffle);

    /// \brief make gaps in vector of Hits
    /// Removes Hits to create one or more Gaps
    /// Currently just drops Hits after the first one. If more gaps
    /// are specified than the number of Hits allow, no Hits are returned.
    std::vector<Hit> & makeGaps(std::vector<Hit> & Hits, uint8_t Gaps);

    /// \brief debug and testing function
    void printHits();

    /// \brief debug and testing function
    void printEvents();

    /// \brief getter for Hits
    std::vector<Hit> & getHits() { return Hits; }

    /// \brief getter for Events
    std::vector<NeutronEvent> & getEvents() { return Events; }

  private:
    /// \brief Convert degrees to radians
    float D2R(float Degrees) {
      auto res = 2 * M_PI * Degrees / 360.0;
      return res;
    }

    uint64_t T0{0}; // Start time for Hits (ns)
    uint32_t TGap{100}; // ns between Events
    uint32_t DeltaT{1}; // ns between Hits
    int CoordMin{0};
    int CoordMax{1279};

    std::vector<Hit> Hits;
    std::vector<NeutronEvent> Events;
    std::mt19937 RandGen; ///< mersenne twister random number generator
    const uint8_t PlaneX{0};
    const uint8_t PlaneY{1};
  };
} // namespace
