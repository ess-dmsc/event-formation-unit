// Copyright (C) 2018 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file GapMatcher.h
/// \brief GapMatcher class definition
///
//===----------------------------------------------------------------------===//

#pragma once


#include <fmt/format.h>
#include <common/reduction/matching/AbstractMatcher.h>

/// \class GapMatcher GapMatcher.h
/// \brief Matcher implementation that joins clusters into events
///         if time gaps between them are sufficiently small.

struct GapMatcherStats {

  /// !!! WHEN ADDING A NEW VARIABLE HERE, ALSO ADD TO METHODS BELOW !!!
  int64_t SpanTooLarge{0};
  int64_t DiscardedSpanTooLarge{0};
  int64_t SplitSpanTooLarge{0};
  int64_t MatchAttemptCount{0};

  /// \brief adds the stats from the passed in GapMatcherStats struct
  /// to this struct, and then clears the stats in the passed in struct
  /// to 0, removing ability to accidentally count these stats multiple times
  void addAndClear(GapMatcherStats &other) {
    SpanTooLarge += other.SpanTooLarge;
    DiscardedSpanTooLarge += other.DiscardedSpanTooLarge;
    SplitSpanTooLarge += other.SplitSpanTooLarge;
    MatchAttemptCount += other.MatchAttemptCount;
    other.clear();
  }

  void clear() {
    SpanTooLarge = 0;
    DiscardedSpanTooLarge = 0;
    SplitSpanTooLarge = 0;
    MatchAttemptCount = 0;
  }
};

class GapMatcher : public AbstractMatcher {
public:
  /// Inherits constructor
  using AbstractMatcher::AbstractMatcher;

  /// \brief sets the maximum time gap criterion
  /// \param max_time_gap maximum time gap between subsequent clusters for
  ///         them to be clustered into one event. If time gap is
  ///         larger, the clusters are split into two events.
  void setMaximumTimeGap(uint64_t max_time_gap);

  /// \brief sets the split multiple simultaneous events functionality
  /// \param split_multi_events boolean, set true to split multiple events,
  ///         false to discard them.
  /// \param coefficient_low lower bound to match clusters in plane 0 and
  ///         plane 1. match is true if sum of ADCs in plane 0 * coefficient_low
  ///         <= sum of ADCs in plane 1.
  /// \param coefficient_high upper bound to match clusters in plane 0 and
  ///         plane 1. match is true if sum of ADCs in plane 0 *
  ///         coefficient_high
  ///         >= sum of ADCs in plane 1.
  void setSplitMultiEvents(bool split_multi_events, float coefficient_low,
                           float coefficient_high);

  /// \brief Match queued up clusters into events.
  ///         Clusters that either overlap in time or have time gaps that are
  ///         smaller than the maximum time gap are joined into events.
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations.
  void match(bool flush) override;

  /// \brief print configuration of GapMatcher
  std::string config(const std::string &prepend) const override;

  struct GapMatcherStats Stats;

private:
  void splitAndStashEvent(Event evt);
  void splitCluster(Cluster cluster, Cluster *new_cluster_1,
                    Cluster *new_cluster_2);
  bool clustersMatch(Cluster cluster_a, Cluster cluster_b);
  void checkAndStashEvent(Event evt);

  uint16_t minimum_coord_gap_{10};
  uint16_t maximum_coord_span_{10};
  float_t coefficient_low_{1};
  float_t coefficient_high_{1};
  bool split_multi_events_{false};
  uint64_t max_time_gap_;
};
