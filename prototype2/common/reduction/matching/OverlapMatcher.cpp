/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file OverlapMatcher.h
/// \brief OverlapMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/matching/OverlapMatcher.h>
#include <common/Trace.h>
#include <cmath>
#include <algorithm>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void OverlapMatcher::match(bool flush) {
  unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.time_start() < c2.time_start();
  });

  XTRACE(CLUSTER, DEB, "match(): unmatched clusters %u", unmatched_clusters_.size());

  Event evt{PlaneA, PlaneB};
  while (!unmatched_clusters_.empty()) {

    auto cluster = unmatched_clusters_.begin();

    if (!flush && !ready_to_be_matched(*cluster)) {
      XTRACE(CLUSTER, DEB, "not ready to be matched");
      break;
    }

    if (!evt.empty() && !evt.time_overlap(*cluster)) {
      XTRACE(CLUSTER, DEB, "no time overlap");
      stash_event(evt);
      evt.clear();
    }

    evt.merge(*cluster);

    unmatched_clusters_.pop_front();
  }

  /// If anything remains
  if (!evt.empty()) {
    if (flush) {
      /// If flushing, stash it
      stash_event(evt);
    } else {
      /// Else return to queue
      // \todo this needs explicit testing
      if (!evt.ClusterA.empty())
        unmatched_clusters_.push_front(evt.ClusterA);
      if (!evt.ClusterB.empty())
        unmatched_clusters_.push_front(evt.ClusterB);
    }
  }
}
