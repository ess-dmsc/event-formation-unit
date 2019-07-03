/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file HitVector.h
/// \brief HitVector alias and convenience functions
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/Hit.h>

using HitVector = std::vector<Hit>;

/// \brief convenience function for sorting Hits by increasing time
inline void sort_chronologically(HitVector &hits) {
  std::sort(hits.begin(), hits.end(),
            [](const Hit &hit1, const Hit &hit2) {
              return hit1.time < hit2.time;
            });
}

/// \brief convenience function for sorting Hits by increasing coordinate
inline void sort_by_increasing_coordinate(HitVector &hits) {
  std::sort(hits.begin(), hits.end(),
            [](const Hit &hit1, const Hit &hit2) {
              return hit1.coordinate < hit2.coordinate;
            });
}

/// \brief convenience function for sorting Hits by decreasing weight
inline void sort_by_decreasing_weight(HitVector &hits) {
  std::sort(hits.begin(), hits.end(),
            [](const Hit &hit1, const Hit &hit2) {
              return hit1.weight > hit2.weight;
            });
}

