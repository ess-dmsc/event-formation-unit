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


static constexpr uint64_t latency{2010}; // ns == 2.01us
// expect readouts in a plane to be at least this close
static constexpr uint64_t timegap{2000};
static constexpr uint64_t coordgap{1}; // allow no gaps between channels

const uint8_t PlaneX{0};
const uint8_t PlaneY{1};

class EventBuilder2D {
public:
  EventBuilder2D();

  // \todo pass by rvalue?
  void insert(Hit hit);

  void flush();

  void clear();

  void setTimeBox(uint32_t TimeBoxValue) { TimeBoxSize = TimeBoxValue; }

  HitVector HitsX, HitsY;

  // \todo parametrize
  GapClusterer ClustersX{timegap, coordgap}, ClustersY{timegap, coordgap};

  // \todo parametrize
  GapMatcher matcher{latency, PlaneX, PlaneY};

  // final vector of reconstructed events
  std::vector<Event> Events;

  // Support for new timebox based clustering (Oct 2020)
  uint64_t TimeBoxT0{0};
  uint32_t TimeBoxSize{10000000};

}; // class

