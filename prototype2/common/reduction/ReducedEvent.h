/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
//===----------------------------------------------------------------------===//
///
/// \file ReducedEvent.h
/// \brief ReducedHit and ReducedEvent class definitions
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>
#include <limits>

/// \class ReducedHit ReducedHit.h
/// \brief Contains the results of a 1-dimensional Cluster analysis.
///        Conceptually this is some coordinate in one dimension, derived by
///        some averaging of Hit coordinates in a Cluster possibly filtered
///        by some prioritization criteria.
///        The exact meaning of each variable is dependent on
///        the precise cluster/event analysis implementation. Not all fields
///        are mandatory and values may be omitted for performance reasons.

struct ReducedHit {
  /// time of cluster
  uint64_t time{0};

  /// average time of hits
  double average_time{std::numeric_limits<double>::quiet_NaN()};

  /// average coordinate of hits
  double center{std::numeric_limits<double>::quiet_NaN()};

  /// span of averaged hit coordinates (narrower uncertainty estimate)
  int16_t uncert_lower{-1};

  /// span of relevant hit coordinates (broader uncertainty estimate)
  int16_t uncert_upper{-1};

  /// number of hits used in average
  size_t hits_used{0};

  /// \returns true if calculated center is finite and non-negative
  bool is_center_good() const;

  /// \returns calculated and rounded coordinate for pixel id
  uint32_t center_rounded() const;

  /// \brief prints values for debug purposes
  std::string to_string() const;
};


/// \class ReducedEvent ReducedEvent.h
/// \brief Contains the results of a multi-dimensional Event analysis.
///        This is a tuple of 1-dimensional analysis results, plus
///        a selected over-all time-point for the entire event, and
///        some evaluation of event validity. All fields are optional
///        and dependent on particular Analysis implementation used.

struct ReducedEvent {
  /// Analysis results for clusters in up to 3 logical dimensions
  ReducedHit x, y, z;

  /// Some selected over-all time point for the entire event
  uint64_t time{0};

  /// Some evaluation of total event validity
  bool good{false};

  /// \brief prints values for debug purposes
  std::string to_string() const;
};
