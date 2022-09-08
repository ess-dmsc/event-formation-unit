/* Copyright (C) 2018 - 2022 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file GapMatcher.h
/// \brief GapMatcher class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/matching/AbstractMatcher.h>

/// \class GapMatcher GapMatcher.h
/// \brief Matcher implementation that joins clusters into events
///         if time gaps between them are sufficiently small.

class GapMatcher : public AbstractMatcher {
public:
  /// Inherits constructor
  using AbstractMatcher::AbstractMatcher;

  /// \brief sets the minimum time gap criterion
  /// \param minimum_time_gap minimum time gap between subsequent clusters for
  /// them
  ///         to be disambiguated into separate events. If time gap is smaller,
  ///         the clusters are merged into one event.
  void set_minimum_time_gap(uint64_t minimum_time_gap);

  
  void set_split_multi_events(bool split_multi_events);

  /// \brief Match queued up clusters into events.
  ///         Clusters that either overlap in time or have time gaps that are
  ///         smaller than the minimum time gap are joined into events.
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations.
  void match(bool flush) override;

  /// \brief print configuration of GapMatcher
  std::string config(const std::string &prepend) const override;

private:
private:
  void split_and_stash_event(Event evt);
  std::vector<Cluster> split_cluster(Cluster cluster);
  bool clusters_match(Cluster cluster_a, Cluster cluster_b);
  void check_and_stash_event(Event evt);

  uint64_t minimum_time_gap_{0};
  uint16_t minimum_coord_gap_{10};
  uint16_t maximum_coord_span_{10};
  float_t coefficient_{1};
  float_t allowance_{10};
  bool split_multi_events_{false};
};
