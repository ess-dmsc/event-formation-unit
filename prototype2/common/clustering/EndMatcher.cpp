/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <common/clustering/EndMatcher.h>

#include <cmath>
#include <algorithm>

EndMatcher::EndMatcher(uint64_t max_delta_time, uint64_t latency)
    : AbstractMatcher(latency), max_delta_time_(max_delta_time) {}

EndMatcher::EndMatcher(uint64_t max_delta_time, uint64_t latency,
                       uint8_t plane1, uint8_t plane2)
    : AbstractMatcher(latency, plane1, plane2), max_delta_time_(max_delta_time) {}

uint64_t EndMatcher::delta_end(const Event &event, const Cluster &cluster) const {
  if (event.time_end() > cluster.time_end())
    return event.time_end() - cluster.time_end();
  return cluster.time_end() - event.time_end();
}

bool EndMatcher::belongs_end(const Event &event, const Cluster &cluster) const {
  return (delta_end(event, cluster) <= max_delta_time_);
}

void EndMatcher::match(bool force) {
  unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.time_end() < c2.time_end();
  });

  Event evt;
  while (!unmatched_clusters_.empty()) {

    auto cluster = unmatched_clusters_.begin();

    if (!force && !ready_to_be_matched(*cluster))
      break;

    if (!evt.empty() && !belongs_end(evt, *cluster)) {
      stash_event(evt);
      evt.clear();
    }

    evt.merge(*cluster);
    unmatched_clusters_.pop_front();
  }

  // If anything is left, stash it
  if (!evt.empty()) {
    stash_event(evt);
  }
}
