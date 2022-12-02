/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
///
/// \file EndMatcher.h
/// \brief EndMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/matching/EndMatcher.h>

#include <algorithm>
#include <cmath>

void EndMatcher::set_max_delta_time(uint64_t max_delta_time) {
  max_delta_time_ = max_delta_time;
}

uint64_t EndMatcher::delta_end(const Event &event,
                               const Cluster &cluster) const {
  if (event.timeEnd() > cluster.timeEnd())
    return event.timeEnd() - cluster.timeEnd();
  return cluster.timeEnd() - event.timeEnd();
}

bool EndMatcher::belongs_end(const Event &event, const Cluster &cluster) const {
  return (delta_end(event, cluster) <= max_delta_time_);
}

void EndMatcher::match(bool flush) {
  unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
    return c1.timeEnd() < c2.timeEnd();
  });

  Event evt{PlaneA, PlaneB};
  while (!unmatched_clusters_.empty()) {

    auto cluster = unmatched_clusters_.begin();

    if (!flush && !ready_to_be_matched(*cluster))
      break;

    if (!evt.empty() && !belongs_end(evt, *cluster)) {
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

std::string EndMatcher::config(const std::string &prepend) const {
  std::stringstream ss;
  ss << AbstractMatcher::config(prepend);
  ss << prepend << fmt::format("max_delta_time: {}\n", max_delta_time_);
  return ss.str();
}
