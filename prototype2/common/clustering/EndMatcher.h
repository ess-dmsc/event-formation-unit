/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file EndMatcher.h
/// \brief EndMatcher class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/AbstractMatcher.h>

/// \class EndMatcher EndMatcher.h
/// \brief Matcher implementation that joins clusters into events
///         if their end times are in close proximity.

class EndMatcher : public AbstractMatcher {
public:
  /// Inherit constructor
  using AbstractMatcher::AbstractMatcher;

  // \todo document this
  void set_max_delta_time(uint64_t max_delta_time);

  /// \brief Match queued up clusters into events.
  ///         Clusters with end times within max_delta_time are joined.
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations
  void match(bool flush) override;

  /// \brief print configuration of EndMatcher
  std::string config(const std::string& prepend) const override;

private:
  uint64_t max_delta_time_{0};

  uint64_t delta_end(const Event &event, const Cluster &cluster) const;
  bool belongs_end(const Event &event, const Cluster &cluster) const;
};
