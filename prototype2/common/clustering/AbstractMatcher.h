/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file AbstractMatcher.h
/// \brief AbstractMatcher class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/AbstractClusterer.h>
#include <common/clustering/Event.h>
#include <deque>

/// \class AbstractMatcher AbstractMatcher.h
/// \brief AbstractMatcher declares the interface for a matcher class.
///         that should match clusters into events. Provides base functionality
///         for storage of clusters and stats counter. Other pre- and
///         post-conditions are left to the discretion of a specific
///         implementation.

class AbstractMatcher {
public:
  AbstractMatcher(uint64_t latency)
      : latency_(latency) {}
  AbstractMatcher(uint64_t latency, uint8_t plane1, uint8_t plane2)
      : latency_(latency), plane1_(plane1), plane2_(plane2) {}
  virtual ~AbstractMatcher() = default;

  /// \brief insert new clusters, queueing them up for matching
  /// \param plane identified plane of all clusters in container
  /// \param other container of clusters in one plane
  void insert(uint8_t plane, ClusterContainer &c) {
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

  /// \brief match queued up clusters into events
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations
  virtual void match(bool flush) = 0;

  std::deque<Event> matched_clusters;
  size_t stats_cluster_count{0};
  /// \todo discarded, other counters?
protected:
  uint64_t latency_{0};
  uint8_t plane1_{0};
  uint8_t plane2_{1};

  ClusterContainer unmatched_clusters_;
  uint64_t latest_x_{0};
  uint64_t latest_y_{0};


  void stash_event(Event& event) {
    matched_clusters.emplace_back(std::move(event));
    stats_cluster_count++;
  }

  bool ready_to_be_matched(const Cluster& cluster) const {
    return ((unmatched_clusters_.size() > 2) &&
        (std::min(latest_x_, latest_y_) - cluster.time_end()) > latency_);
  }

};
