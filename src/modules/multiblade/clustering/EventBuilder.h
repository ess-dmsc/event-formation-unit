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
};

}
