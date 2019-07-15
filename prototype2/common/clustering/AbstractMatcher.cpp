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

AbstractMatcher::AbstractMatcher(uint64_t maximum_latency, uint8_t planeA, uint8_t planeB)
    : maximum_latency_(maximum_latency), PlaneA(planeA), PlaneB(planeB) {}

void AbstractMatcher::insert(const Cluster &cluster) {
  if (cluster.plane() == PlaneA) {
    LatestA = std::max(LatestA, cluster.time_start());
  } else if (cluster.plane() == PlaneB) {
    LatestB = std::max(LatestB, cluster.time_start());
  } else {
    stats_rejected_clusters++;
    return;
  }
  unmatched_clusters_.push_back(cluster);
}

void AbstractMatcher::insert(const ClusterContainer &clusters) {
  for (auto &cluster : clusters)
    insert(cluster);
}

void AbstractMatcher::insert(uint8_t plane, ClusterContainer &clusters) {
  if (clusters.empty()) {
    return;
  }
  if (plane == PlaneA) {
    LatestA = std::max(LatestA, clusters.back().time_start());
  } else if (plane == PlaneB) {
    LatestB = std::max(LatestB, clusters.back().time_start());
  } else {
    stats_rejected_clusters++;
    return;
  }
  unmatched_clusters_.splice(unmatched_clusters_.end(), clusters);
}

void AbstractMatcher::stash_event(Event &event) {
  matched_events.emplace_back(std::move(event));
  stats_event_count++;
}

bool AbstractMatcher::ready_to_be_matched(const Cluster &cluster) const {
  XTRACE(CLUSTER, DEB,
         "latest_x %u, latest_y %u, cl time end %u", LatestA, LatestB, cluster.time_end());
  auto latest = std::min(LatestA, LatestB);
  return (latest > cluster.time_end()) &&
      ((latest - cluster.time_end()) > maximum_latency_);
}

std::string AbstractMatcher::config(const std::string &prepend) const {
  std::stringstream ss;
  ss << prepend << fmt::format("latency: {}\n", maximum_latency_);
  ss << prepend << fmt::format("PlaneA: {}\n", PlaneA);
  ss << prepend << fmt::format("PlaneB: {}\n", PlaneB);
  return ss.str();
}

std::string AbstractMatcher::status(const std::string &prepend, bool verbose) const {
  std::stringstream ss;
  ss << prepend << fmt::format("stats_event_count: {}\n", stats_event_count);
  ss << prepend << fmt::format("stats_rejected_clusters: {}\n", stats_rejected_clusters);
  if (!matched_events.empty()) {
    ss << prepend << "Matched events:\n";
    // \todo factor this out
    for (const auto &e : matched_events) {
      ss << prepend << "  " << e.to_string(prepend + "  ", verbose) + "\n";
    }
  }
  ss << prepend << fmt::format("latest in PlaneA: {}\n", LatestA);
  ss << prepend << fmt::format("latest in PlaneB: {}\n", LatestB);
  ss << prepend << "Unmatched clusters:\n"
     << to_string(unmatched_clusters_, prepend + "  ", verbose);
  return ss.str();
}

