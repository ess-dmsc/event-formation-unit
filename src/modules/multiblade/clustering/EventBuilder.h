// Copyright (C) 2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Time-boxed event builder for Multi-Blade
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/clustering/GapClusterer.h>
#include <common/reduction/matching/GapMatcher.h>

namespace Multiblade {

/// \todo these tme values should be in ns. At the moment they are
/// given in 'ticks'
static constexpr uint64_t latency{2010}; // 2.01us
static constexpr uint64_t timegap{2000};  // expect readouts in a plane to be at least this close
static constexpr uint64_t coordgap{1};  // allow no gaps between channels

const uint8_t WirePlane{0};
const uint8_t StripPlane{1};

class EventBuilder {
public:
  EventBuilder();

  explicit EventBuilder(uint32_t BoxSize);

  // Check adc values against supplied thresholds
  // \todo Should be moved out of here
  bool thresholdCheck(uint8_t DigIndex, uint16_t GlobalChannel, uint16_t AdcValue);

  // \todo pass by rvalue?
  void insert(Hit hit);

  void flush();

  void clear();

  HitVector p0, p1;

  // \todo parametrize
  GapClusterer c0{timegap, coordgap}, c1{timegap, coordgap};

  // \todo parametrize
  GapMatcher matcher{latency, WirePlane, StripPlane};

  // final vector of reconstructed events
  std::vector<Event> Events;

  // Support for new timebox based clustering (Oct 2020)
  uint64_t TimeBoxT0{0};
  uint32_t TimeBoxSize{10000000};

  // Data from MB18_thresholds_sw36.xlsx
  // only for the lower (?) 32 channels per digitizer
  std::vector<std::vector<uint16_t>> Thresholds {
    { 34,  5000,  5000,  5000,  5000,  5000,  5000, 10500,  9000,
          12000,  5000, 12500, 13000, 12500,  8000,  9143, 10500,
           8000,  8000,  8000,  8000, 10000, 10000,  5000,  8034,
           6000,  6000,  6000,  6000,  6000,  6000,  6000, 20000},

    { 33, 12381, 16190, 14762, 14762, 17143, 13333, 15238, 15873,
          15504, 15152, 14815, 15942, 15603, 15238, 12571, 14667,
          14379, 12821, 12579, 13580, 13810, 13095, 12865, 12644,
          12429, 13333, 12022, 11828, 11640, 11458, 11282, 20000},

    {137,  5000, 14000, 13000, 14000, 19000, 12000, 17073, 16667,
          16279, 15909, 15000, 15217, 14894, 14583, 14286, 15000,
          13725, 13462, 13208, 14000, 12727, 12500, 12281, 12069,
          11864, 11667, 11475, 11290, 11111, 10938, 10769, 20000},

    {142,  5000, 15000, 14000, 15000, 16000, 10000, 15000, 17000,
          16279, 15909, 15556, 15217, 14894, 15500, 14286, 13000,
          13725, 13462, 13208, 14000, 12727, 12500, 12281, 12069,
          11864, 11667, 11475, 11290, 11111, 10938, 10769, 20000},

    {143,  7000,  7000,  7000,  7000,  7000,  7000, 12195, 11905,
          11628, 11364, 11111, 12000, 14000, 14000, 12000, 12000,
          12000, 12000, 12500, 12000, 11000, 12500, 11000, 12500,
          12000, 13000,  9000,  9000,  9000,  9000,  9000, 20000},

    { 31, 10500, 18500, 19500, 19500, 21500, 15000, 19744, 19274,
          18826, 18398, 18889, 18478, 18500, 18500, 16500, 18000,
          17000, 15000, 15000, 15500, 18500, 15179, 13000, 13500,
          14407, 14167, 15000, 13710, 13492, 13281, 13077, 20000},
  };

}; // class

} // namespace
