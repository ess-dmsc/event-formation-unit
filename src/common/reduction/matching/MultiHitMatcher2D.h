/* Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file MultiHitMatcher2D.h
/// \brief MultiHitMatcher2D class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/matching/AbstractMatcher.h>

/// \class MultiHitMatcher2D MultiHitMatcher2D.h
/// \brief Matcher implementation that joins clusters into events
///         if time gaps between them are sufficiently small, and
///         handles multiple hits in a time window by splitting
///         clusters by coordinate gaps and using relationship
///         between ADC values in different planes to match
///         resulting clusters

class MultiHitMatcher2D : public AbstractMatcher {

public:
  /// Inherits constructor
  using AbstractMatcher::AbstractMatcher;
  /// \brief sets the minimum time gap criterion
  /// \param minimum_time_gap minimum time gap between subsequent clusters for
  /// them
  ///         to be disambiguated into separate events. If time gap is smaller,
  ///         the clusters are merged into one event.
  void set_minimum_time_gap(uint64_t minimum_time_gap);

  /// \brief Match queued up clusters into events.
  ///         Clusters that either overlap in time or have time gaps that are
  ///         smaller than the minimum time gap are joined into events.
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations.
  void match(bool flush) override;

  /// \brief print configuration of GapMatcher
  std::string config(const std::string &prepend) const override;

private:
  void split_and_stash_event(Event evt);
  std::vector<Cluster> split_cluster(Cluster cluster);
  bool clusters_match(Cluster cluster_a, Cluster cluster_b);

  uint64_t minimum_time_gap_{0};
  uint16_t minimum_coord_gap_{10};
  uint16_t maximum_coord_span_{10};
  float_t coefficient_{1};
  float_t allowance_{10};
};
