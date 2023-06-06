// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
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

  /// \brief Check for overlapping intevals in a list
  /// First put intervals into canonical order where the first element is
  /// smaller than the second (for calibration the reversal is ok), then sort
  /// the intervals in ascending first elements. Finally, iterate through
  /// the list and compare the start of the iterator with the end of the
  /// previous interval.
  bool Interval::overlaps(std::vector<std::pair<double, double>> Intervals) {
    /// put intervals into canonical order (first < second)
    for (auto & Interval : Intervals) {
      if (Interval.second < Interval.first) {
        double tmp = Interval.first;
        Interval.first = Interval.second;
        Interval.second = tmp;
      }
    }

    std::sort(Intervals.begin(), Intervals.end(), Compare);
    //print(Intervals);

    std::vector<std::pair<double, double>> ret;
    for (auto &it : Intervals) {
      if (ret.empty()) {
        ret.push_back(it);
      } else {
        if ((ret.back().second - it.first) > -EPSILON) {
          return true;
        } else {
          ret.push_back(it);
        }
      }
    }
    return false;
  }

} // namespace Caen
