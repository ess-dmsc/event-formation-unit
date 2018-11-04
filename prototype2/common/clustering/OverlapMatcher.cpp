/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <common/clustering/OverlapMatcher.h>

#include <cmath>
#include <algorithm>

OverlapMatcher::OverlapMatcher(uint64_t latency)
    : latency_(latency) {}

OverlapMatcher::OverlapMatcher(uint64_t latency, uint8_t plane1, uint8_t plane2)
    : latency_(latency), plane1_(plane1), plane2_(plane2) {}

bool OverlapMatcher::ready_to_be_matched(double time) const {
  return ((unmatched_clusters_.size() > 2) &&
      (std::min(latest_x_, latest_y_) - time) > latency_);
}

void OverlapMatcher::insert(uint8_t plane, ClusterContainer &c) {
  if (c.empty()) {
    return;
  }
  if (plane == plane1_) {
    latest_x_ = std::max(latest_x_, c.back().time_start());
  } else if (plane == plane2_) {
    latest_y_ = std::max(latest_y_, c.back().time_start());
  }
  unmatched_clusters_.splice(unmatched_clusters_.end(), c);
}

void OverlapMatcher::match(bool flush) {
  unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.time_end() < c2.time_end();
  });

  Event evt;
  while (!unmatched_clusters_.empty()) {

    auto cluster = unmatched_clusters_.begin();

    if (!flush && !ready_to_be_matched(cluster->time_end()))
      break;

    if (!evt.empty() && !evt.time_overlap(*cluster)) {
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
