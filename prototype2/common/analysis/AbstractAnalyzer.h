/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Classes for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/Event.h>
#include <limits>

namespace Gem
{

struct CoordResult {
  double center{std::numeric_limits<double>::quiet_NaN()}; // entry strip
  int16_t uncert_lower{-1}; /// strip span of hits in latest timebin
  int16_t uncert_upper{-1}; /// strip span of hits in latest few timebins

  /// \brief returns calculated and rounded entry strip number for pixid
  uint32_t center_rounded() const;

  /// \brief prints values for debug purposes
  std::string debug() const;
};

struct MultiDimResult {
  CoordResult x, y, z;

  uint64_t time {0};
  bool good {false};

  std::string debug() const;
};

class AbstractAnalyzer {
 public:
  AbstractAnalyzer() = default;
  virtual ~AbstractAnalyzer() = default;

  /// \brief analyzes cluster in both planes
  virtual MultiDimResult analyze(Event&) const = 0;

  virtual std::string debug() const = 0;
};

}
