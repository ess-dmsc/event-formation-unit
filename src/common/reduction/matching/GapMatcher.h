/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
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

  /// \brief Match queued up clusters into events.
  ///         Clusters that either overlap in time or have time gaps that are
  ///         smaller than the minimum time gap are joined into events.
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations.
  void match(bool flush) override;

  /// \brief print configuration of GapMatcher
  std::string config(const std::string &prepend) const override;

private:
  uint64_t minimum_time_gap_{0};
};
