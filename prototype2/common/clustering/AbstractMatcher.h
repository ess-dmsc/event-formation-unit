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
/// \brief AbstractMatcher Declares the interface for a matcher class.
///         that should match clusters into events. Provides base functionality
///         for storage of clusters and stats counter. Other pre- and
///         post-conditions are left to the discretion of a specific
///         implementation.

class AbstractMatcher {
public:
  /// \brief AbstractMatcher constructor
  /// \param latency Minimum time gap to latest cluster for a cluster to be
  ///         considered for matching. Of the latest clusters in both planes,
  ///         the earlier one will be considered for this comparison.
  /// \param plane1 id of first plane selected for matching
  /// \param plane2 id of second plane selected for matching
  AbstractMatcher(uint64_t latency, uint8_t plane1, uint8_t plane2);

  /// \brief AbstractMatcher constructor
  /// \param latency Minimum time gap to latest cluster for a cluster to be
  ///         considered for matching. Of the latest clusters in both planes,
  ///         the earlier one will be considered for this comparison.
  AbstractMatcher(uint64_t latency);

  virtual ~AbstractMatcher() = default;

  // \todo insert individual clusters?

  /// \brief insert new clusters, queueing them up for matching
  /// \param plane identified plane of all clusters in container
  /// \param other container of clusters in one plane. Clusters must be
  ///         chronological to the extent at the latency guarantee holds, i.e.
  ///         if container contains an cluster with start_time=T, then no
  ///         clusters can arrive with end_time>=(T-latency).
  void insert(uint8_t plane, ClusterContainer &c);

  /// \brief match queued up clusters into events
  ///         To be implemented in derived classes.
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations
  virtual void match(bool flush) = 0;

  std::deque<Event> matched_events;
  size_t stats_event_count{0}; ///< cumulative number of matched events
  // \todo discarded, other counters?

protected:
  uint64_t latency_{0}; ///< time gap for a cluster to be considered for matching
  uint8_t plane1_{0};
  uint8_t plane2_{1};

  ClusterContainer unmatched_clusters_;
  uint64_t latest_x_{0};
  uint64_t latest_y_{0};

  /// \brief Moves event into events container; increments counter.
  /// \param event to be stashed
  void stash_event(Event& event);

  /// \brief Determines if cluster is ready to be matched. Compares the end time of
  ///         submitted cluster to the start time of the latest cluster in queue.
  ///         Of the latest clusters in both planes, the earlier one will be
  ///         considered. Uses the latency criterion as threshold.
  /// \param cluster to be considered for matching
  bool ready_to_be_matched(const Cluster& cluster) const;
};
