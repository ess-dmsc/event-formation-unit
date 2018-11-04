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
    : max_delta_time_(max_delta_time), latency_(latency) {}

EndMatcher::EndMatcher(uint64_t max_delta_time, uint64_t latency,
                       uint8_t plane1, uint8_t plane2)
    : max_delta_time_(max_delta_time), latency_(latency),
      plane1_(plane1), plane2_(plane2) {}

bool EndMatcher::ready_to_be_matched(double time) const {
  /// \todo Parametrize threshold
  return ((unmatched_clusters_.size() > 2) &&
      (std::min(latest_x_, latest_y_) - time) > latency_);
}

uint64_t EndMatcher::delta_end(const Event &event, const Cluster &cluster) const {
  if (event.time_end() > cluster.time_end())
    return event.time_end() - cluster.time_end();
  return cluster.time_end() - event.time_end();
}

bool EndMatcher::belongs_end(const Event &event, const Cluster &cluster) const {
  return (delta_end(event, cluster) <= max_delta_time_);
}

void EndMatcher::insert(uint8_t plane, ClusterContainer &c) {
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

void EndMatcher::match(bool force) {
  /// \todo is it already sorted?
  unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.time_end() < c2.time_end();
  });

  Event evt;

  while (!unmatched_clusters_.empty()) {

    auto n = unmatched_clusters_.begin();

    if (!force && !ready_to_be_matched(n->time_end()))
      break;

    if (!evt.empty() && !belongs_end(evt, *n)) {
      stash_event(evt);
      evt = Event();
    }

    evt.merge(*n);
    unmatched_clusters_.pop_front();
  }

  // If anything is left, stash it
  if (!evt.empty()) {
    stash_event(evt);
  }
}
