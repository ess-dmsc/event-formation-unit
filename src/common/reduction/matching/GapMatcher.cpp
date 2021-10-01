/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file GapMatcher.h
/// \brief GapMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/matching/GapMatcher.h>
#include <common/debug/Trace.h>
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

  Event evt{PlaneA, PlaneB};
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

  /// If anything remains
  if (!evt.empty()) {
    if (flush) {
      stash_event(evt);
    } else {
      requeue_clusters(evt);
    }
  }
}

std::string GapMatcher::config(const std::string& prepend) const {
  std::stringstream ss;
  ss << AbstractMatcher::config(prepend);
  ss << prepend << fmt::format("minimum_time_gap: {}\n", minimum_time_gap_);
  return ss.str();
}

