// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <modules/caen/geometry/Interval.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

/// \brief Check for overlapping intervals in a list
/// First put intervals into canonical order where the first element is
/// smaller than the second (for calibration the reversal is ok). Then sort
/// the intervals in ascending first elements. Finally, iterate through
/// the list and compare the start of the iterator with the end of the
/// previous interval.
bool Interval::overlaps(std::vector<std::pair<double, double>> Intervals) {
  /// Put intervals into canonical order (first < second)
  for (auto &[i0, i1] : Intervals) {
    if (i1 < i0) {
      std::swap(i0, i1);
    }
  }

  // Sort intervals and check if the end point of the i'th interval is larger
  // than the start point of the next interval
  std::sort(Intervals.begin(), Intervals.end(), Compare);
  for (size_t i=1; i < Intervals.size(); i++) {
    if (Intervals[i-1].second - Intervals[i].first > -EPSILON) {
      return true;
    }
  }

  return false;
}

} // namespace Caen
