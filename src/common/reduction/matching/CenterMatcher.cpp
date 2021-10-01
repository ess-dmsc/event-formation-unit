// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file CenterMatcher.h
/// \brief CenterMatcher class implementation
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/reduction/matching/CenterMatcher.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

void CenterMatcher::set_max_delta_time(uint64_t max_delta_time) {
  max_delta_time_ = max_delta_time;
}

void CenterMatcher::set_time_algorithm(std::string time_algorithm) {
  time_algorithm_ = time_algorithm;
}

void CenterMatcher::match(bool flush) {

  if (time_algorithm_ == "center-of-mass") {
    unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
      return c1.time_center() < c2.time_center();
    });
  } else if (time_algorithm_ == "charge2") {
    unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
      return c1.time_center2() < c2.time_center2();
    });
  }
  // time_algorithm_ == "utpc" or time_algorithm_ == "utpc-weighted"
  else {
    unmatched_clusters_.sort([](const Cluster &c1, const Cluster &c2) {
      return c1.time_end() < c2.time_end();
    });
  }

  XTRACE(CLUSTER, DEB, "match(): unmatched clusters %u",
         unmatched_clusters_.size());

  Event evt{PlaneA, PlaneB};

  while (!unmatched_clusters_.empty()) {

    auto cluster = unmatched_clusters_.begin();

    if (!flush && !ready_to_be_matched(*cluster)) {
      XTRACE(CLUSTER, DEB, "not ready to be matched");
      break;
    }

    // if the event is complete in both planes, stash it
    if (evt.both_planes()) {
      XTRACE(CLUSTER, DEB, "stash complete plane1/2 event");
      stash_event(evt);
      evt.clear();
    }
    if (!evt.empty()) {
      if (evt.time_gap(*cluster) > max_delta_time_) {
        XTRACE(CLUSTER, DEB, "time gap too large");
        stash_event(evt);
        evt.clear();
      }
      // Plane 1 has value 0
      if (cluster->plane() == 0) {
        if (!evt.ClusterA.empty()) {
          XTRACE(CLUSTER, DEB, "stash plane 1 event");
          stash_event(evt);
          evt.clear();
        }
      }
      // Plane 2 has value 1
      else if (cluster->plane() == 1) {
        if (!evt.ClusterB.empty()) {
          XTRACE(CLUSTER, DEB, "stash plane 2 event");
          stash_event(evt);
          evt.clear();
        }
      }
    }
    // Add only to the cluster, if the plane is empty
    evt.merge(*cluster);
    unmatched_clusters_.pop_front();
  }

  if (!evt.empty()) {
    if (flush) {
      // If flushing, stash it
      stash_event(evt);
    } else {
      // Else return to queue
      // \todo this needs explicit testing
      if (!evt.ClusterA.empty())
        unmatched_clusters_.push_front(std::move(evt.ClusterA));
      if (!evt.ClusterB.empty())
        unmatched_clusters_.push_front(std::move(evt.ClusterB));
    }
  }
}

std::string CenterMatcher::config(const std::string &prepend) const {
  std::stringstream ss;
  ss << AbstractMatcher::config(prepend);
  ss << prepend << fmt::format("time_algorithm: {}\n", time_algorithm_);
  ss << prepend << fmt::format("max_delta_time: {}\n", max_delta_time_);
  return ss.str();
}
