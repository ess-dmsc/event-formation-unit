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
  AbstractMatcher() = default;
  virtual ~AbstractMatcher() = default;

  /// \brief insert new clusters, queueing them up for matching
  /// \param plane identified plane of all clusters in container
  /// \param other container of clusters in one plane
  virtual void insert(uint8_t plane, ClusterContainer &c) = 0;

  /// \brief match queued up clusters into events
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations
  virtual void match(bool flush) = 0;

  std::deque<Event> matched_clusters;
  size_t stats_cluster_count{0};
  /// \todo discarded, other counters?
protected:
  void stash_event(Event& event) {
    matched_clusters.emplace_back(std::move(event));
    stats_cluster_count++;
  }

};
