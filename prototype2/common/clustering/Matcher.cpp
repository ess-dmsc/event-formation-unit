/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <common/clustering/Matcher.h>

#include <cmath>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

Matcher::Matcher(uint64_t maxDeltaTime) : pMaxDeltaTime(maxDeltaTime) {
}

bool Matcher::ready_to_be_matched(double time) const {
  /// \todo Parametrize threshold
  return ((unmatched_clusters.size() > 2) &&
      (std::min(latest_x, latest_y) - time) > (pMaxDeltaTime * 3));
}

double Matcher::delta_end(const Event &event, const Cluster &cluster) const {
  if (event.time_end() > cluster.time_end())
    return event.time_end() - cluster.time_end();
  return cluster.time_end() - event.time_end();
}

bool Matcher::belongs_end(const Event &event, const Cluster &cluster) const {
  return (delta_end(event, cluster) <= pMaxDeltaTime);
}

void Matcher::insert(uint8_t plane, ClusterContainer &c) {
  if (c.empty()) {
    return;
  }
  if (plane == 1) {
    latest_y = std::max(latest_y, c.back().time_start());
  } else if (plane == 0) {
    latest_x = std::max(latest_x, c.back().time_start());
  }
  unmatched_clusters.splice(unmatched_clusters.end(), c);
}

void Matcher::flush() {
  match_end(true);
}

void Matcher::match_end(bool force) {
  /// \todo is it already sorted?
  unmatched_clusters.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.time_end() < c2.time_end();
  });

  Event evt;

  while (!unmatched_clusters.empty()) {

    auto n = unmatched_clusters.begin();

    if (!force && !ready_to_be_matched(n->time_end()))
      break;

    if (!evt.empty() && !belongs_end(evt, *n)) {
      matched_clusters.emplace_back(std::move(evt));
      stats_cluster_count++;
      evt = Event();
    }

    evt.merge(*n);
    unmatched_clusters.pop_front();
  }

  // If anything is left, stash it
  if (!evt.empty()) {
    matched_clusters.emplace_back(std::move(evt));
    stats_cluster_count++;
  }
}
