// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief class to check for overlapping intervals (used in CDCalibration)
///
// Inspired by various int based examples but adapted to doubles which
// can't be compared directly for equality, so we use EPSILON to imply being
// close enough.
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <fmt/format.h>
#include <utility>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

class Interval {
public:

  ///\brief compare function implemented for use in sort()
  bool static Compare(const std::pair<double, double> &a, const std::pair<double, double> &b) {
    return abs(a.first - b.first) < EPSILON ? a.second > b.second : a.first < b.first;
  }

  ///\brief for debugging
  void static print(std::vector<std::pair<double, double>> Intervals) {
    std::string Msg{""};
    for (auto & Interval : Intervals) {
      Msg += fmt::format("[{}, {}] ", Interval.first, Interval.second);
    }
    XTRACE(INIT, ALW, "%s", Msg.c_str());
  }

  ///\brief check if at least one pair of intervals overlap
  ///\return true if overlapping, else false.
  bool static overlaps(std::vector<std::pair<double, double>> Intervals);

  // interval endpoints cannot be closer than this.
  static constexpr double EPSILON{0.00001};

};
} // namespace Caen
