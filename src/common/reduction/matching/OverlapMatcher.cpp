/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file OverlapMatcher.h
/// \brief OverlapMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <cmath>
#include <common/debug/Trace.h>
#include <common/reduction/matching/OverlapMatcher.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void OverlapMatcher::match(bool flush) {
  unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.timeStart() < c2.timeStart();
  });

  XTRACE(CLUSTER, DEB, "match(): unmatched clusters %u",
         unmatched_clusters_.size());

  Event evt{PlaneA, PlaneB};
  while (!unmatched_clusters_.empty()) {

    auto cluster = unmatched_clusters_.begin();

    if (!flush && !ready_to_be_matched(*cluster)) {
      XTRACE(CLUSTER, DEB, "not ready to be matched");
      break;
    }

    if (!evt.empty() && !evt.timeOverlap(*cluster)) {
      XTRACE(CLUSTER, DEB, "no time overlap");
      stashEvent(evt);
      evt.clear();
    }

    evt.merge(*cluster);

    unmatched_clusters_.pop_front();
  }

  /// If anything remains
  if (!evt.empty()) {
    if (flush) {
      stashEvent(evt);
    } else {
      requeue_clusters(evt);
    }
  }
}
