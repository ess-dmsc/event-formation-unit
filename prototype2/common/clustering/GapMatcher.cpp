/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file GapMatcher.h
/// \brief GapMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <common/clustering/GapMatcher.h>
#include <common/Trace.h>
// #include <cmath>
// #include <algorithm>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void GapMatcher::set_minimum_time_gap(uint64_t minimum_time_gap) {
  minimum_time_gap_ = minimum_time_gap;
}

void GapMatcher::match(bool flush) {
  unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.time_start() < c2.time_start();
  });

  XTRACE(CLUSTER, DEB, "match(): unmatched clusters %u", unmatched_clusters_.size());

  Event evt{plane1_, plane2_};
  while (!unmatched_clusters_.empty()) {

    auto cluster = unmatched_clusters_.begin();

    if (!flush && !ready_to_be_matched(*cluster)) {
      XTRACE(CLUSTER, DEB, "not ready to be matched");
      break;
    }

    if (!evt.empty() && (evt.time_gap(*cluster) > minimum_time_gap_)) {
      XTRACE(CLUSTER, DEB, "time gap too large");
      stash_event(evt);
      evt.clear();
    }

    evt.merge(*cluster);

    unmatched_clusters_.pop_front();
  }

  if (!evt.empty()) {
    if (flush) {
      // If flushing, stash it
      stash_event(evt);
    } else {
      // Else return merged clusters to their respective queue
      // \todo this needs explicit testing
      if (!evt.cluster1.empty())
        unmatched_clusters_.push_front(evt.cluster1);
      if (!evt.cluster2.empty())
        unmatched_clusters_.push_front(evt.cluster2);
    }
  }
}
