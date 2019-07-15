/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>
#include <limits>

struct ReducedHit {
  uint64_t time{0}; /// < time of cluster (implementation-dependent)
  double average_time{std::numeric_limits<double>::quiet_NaN()}; /// < average time of hits
  double center{std::numeric_limits<double>::quiet_NaN()}; /// < average coordinate of hits
  int16_t uncert_lower{-1}; /// < span of averaged hit coordinates
  int16_t uncert_upper{-1}; /// < span of relevant (implementation-dependent) hit coordinates
  size_t hits_used{0}; /// < number of hits used in average

  /// \brief returns calculated and rounded coordinate for pixel id
  uint32_t center_rounded() const;

  bool is_center_good() const;

  /// \brief prints values for debug purposes
  std::string debug() const;
};

struct ReducedEvent {
  ReducedHit x, y, z;

  uint64_t time {0};
  bool good {false};

  /// \returns hits used across all dimensions
  size_t hits_used() const;

  std::string debug() const;
};
