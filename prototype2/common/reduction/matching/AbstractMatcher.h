/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file AbstractMatcher.h
/// \brief AbstractMatcher class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/clustering/AbstractClusterer.h>
#include <common/reduction/Event.h>
#include <deque>

/// \class AbstractMatcher AbstractMatcher.h
/// \brief AbstractMatcher Declares the interface for a matcher class.
///         that should match clusters into events. Provides base functionality
///         for storage of clusters and stats counter. Other pre- and
///         post-conditions are left to the discretion of a specific
///         implementation.

class AbstractMatcher {
public:
  std::vector<Event> matched_events;
  mutable size_t stats_event_count{0}; ///< cumulative number of matched events
  mutable size_t stats_rejected_clusters{0}; ///< cumulative number of rejected clusters
  // \todo more counters?

public:
  /// \brief AbstractMatcher constructor
  /// \param maximum_latency Minimum time gap to latest cluster for a cluster to be
  ///         considered for matching. Of the latest clusters in both planes,
  ///         the earlier one will be considered for this comparison.
  /// \param planeA id of first plane selected for matching
  /// \param planeB id of second plane selected for matching
  AbstractMatcher(uint64_t maximum_latency, uint8_t planeA, uint8_t planeB);

  virtual ~AbstractMatcher() = default;

  /// \brief inserts new cluster, queueing it up for matching
  /// \param cluster Cluster to be inserted. Must belong to one of selected planes,
  ///         otherwise will be rejected (not added to queue).
  /// \note Inserted clusters must be chronological to the extent at the latency
  ///        guarantee holds, i.e. if cluster has start_time=T, then no
  ///        subsequent clusters can arrive with end_time>=(T-latency).
  void insert(const Cluster &cluster);

  /// \brief insert new clusters potentially belonging to multiple planes.
  /// \param clusters container of clusters, sorted chronologically.
  /// \note Inserted clusters must be chronological to the extent at the latency
  ///        guarantee holds, i.e. if cluster has start_time=T, then no
  ///        subsequent clusters can arrive with end_time>=(T-latency).
  void insert(const ClusterContainer &clusters);

  /// \brief insert new clusters belonging to the same plane (optimized).
  /// \param plane identified plane of all clusters in container
  /// \param clusters container of clusters in one plane, sorted chronologically.
  /// \note Inserted clusters must be chronological to the extent at the latency
  ///        guarantee holds, i.e. if cluster has start_time=T, then no
  ///        subsequent clusters can arrive with end_time>=(T-latency).
  void insert(uint8_t plane, ClusterContainer &clusters);

  /// \brief match queued up clusters into events
  ///         To be implemented in derived classes.
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations
  /// \note This is the main function to be implemented by inheriting classes
  virtual void match(bool flush) = 0;

  /// \brief print configuration of Matcher
  virtual std::string config(const std::string &prepend) const;

  /// \brief print current status of Matcher
  virtual std::string status(const std::string &prepend, bool verbose) const;

protected:
  uint64_t maximum_latency_{0}; ///< time gap for a cluster to be considered for matching
  uint8_t PlaneA{0};
  uint8_t PlaneB{1};

  ClusterContainer unmatched_clusters_;
  uint64_t LatestA{0};
  uint64_t LatestB{0};

  /// \brief Moves event into events container; increments counter.
  /// \param event to be stashed
  void stash_event(Event &event);

  /// \brief Puts the event's constituent clusters back onto their respective
  ///         unmatched cluster queues.
  /// \param event to be disassembled
  void requeue_clusters(Event &event);

  /// \brief Determines if cluster is ready to be matched, using maximum latency
  ///         as time-out threshold. Compares the end time of submitted cluster to
  ///         the start time of the latest cluster in queue. Of the latest clusters
  ///         in both planes, the earlier one will be considered. Uses the latency
  ///         criterion as threshold.
  /// \param cluster to be considered for matching
  bool ready_to_be_matched(const Cluster &cluster) const;
};
