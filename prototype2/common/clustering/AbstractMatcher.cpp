/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file AbstractMatcher.cpp
/// \brief AbstractMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <common/clustering/AbstractMatcher.h>
#include <common/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

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

void AbstractMatcher::insert_pulses(HitContainer &hits) {
  if (hits.empty())
    return;
  track_pulses_ = true;
  for (auto& h : hits) {
    h.plane = pulse_plane_;
    Cluster c;
    c.insert(h);
    unmatched_clusters_.push_back(c);
    latest_pulse_ = std::max(latest_pulse_, h.time);
  }
  hits.clear();
}


void AbstractMatcher::stash_event(Event &event) {
  matched_events.emplace_back(std::move(event));
  stats_event_count++;
}

bool AbstractMatcher::ready_to_be_matched(const Cluster &cluster) const {
  XTRACE(CLUSTER, DEB, "latest_x %u, latest_y %u, cl time end %u", latest_x_, latest_y_, cluster.time_end());
  // \todo print info on pulses
  auto latest = std::min(latest_x_, latest_y_);
  if (track_pulses_)
    latest = std::min(latest, latest_pulse_);
  return (latest > cluster.time_end()) &&
      ((latest - cluster.time_end()) > latency_);
}
