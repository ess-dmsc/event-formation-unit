/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file AbstractMatcher.cpp
/// \brief AbstractMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <common/clustering/AbstractMatcher.h>

AbstractMatcher::AbstractMatcher(uint64_t latency)
    : latency_(latency) {}
AbstractMatcher::AbstractMatcher(uint64_t latency, uint8_t plane1, uint8_t plane2)
    : latency_(latency), plane1_(plane1), plane2_(plane2) {}

void AbstractMatcher::insert(uint8_t plane, ClusterContainer &c) {
  if (c.empty()) {
    return;
  }
  if (plane == plane1_) {
    latest_x_ = std::max(latest_x_, c.back().time_start());
  } else if (plane == plane2_) {
    latest_y_ = std::max(latest_y_, c.back().time_start());
  } else {
    return;
  }
  unmatched_clusters_.splice(unmatched_clusters_.end(), c);
}

void AbstractMatcher::stash_event(Event &event) {
  matched_events.emplace_back(std::move(event));
  stats_event_count++;
}

bool AbstractMatcher::ready_to_be_matched(const Cluster &cluster) const {
  auto latest = std::min(latest_x_, latest_y_);
  return (latest > cluster.time_end()) &&
      ((std::min(latest_x_, latest_y_) - cluster.time_end()) > latency_);
}


