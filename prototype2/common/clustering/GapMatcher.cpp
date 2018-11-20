/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file OverlapMatcher.h
/// \brief OverlapMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <common/clustering/GapMatcher.h>
#include <common/Trace.h>
// #include <cmath>
// #include <algorithm>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

GapMatcher::GapMatcher(uint64_t latency, uint64_t time_gap)
    : AbstractMatcher(latency), allowed_time_gap_(time_gap) {}

// OverlapMatcher::OverlapMatcher(uint64_t latency, uint8_t plane1, uint8_t plane2)
//     : AbstractMatcher(latency, plane1, plane2) {}


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

      if (!evt.empty() && !evt.time_overlap(*cluster, allowed_time_gap_)) {
        XTRACE(CLUSTER, DEB, "no time overlap");
        stash_event(evt);
        evt.clear();
      }

      evt.merge(*cluster);
      unmatched_clusters_.pop_front();
    }

  // If anything is left, stash it
  // \todo maybe only on flush? otherwise return to queue?
  if (!evt.empty()) {
    stash_event(evt);
  }
}
