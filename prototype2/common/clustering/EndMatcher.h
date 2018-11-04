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
  /// \brief EndMatcher constructor
  /// \param max_delta_time how close clusters' end times must be to merge
  /// \sa AbstractMatcher
  EndMatcher(uint64_t max_delta_time, uint64_t latency);
  
  /// \brief EndMatcher constructor
  /// \param max_delta_time how close clusters' end times must be to merge
  /// \sa AbstractMatcher
  EndMatcher(uint64_t max_delta_time, uint64_t latency,
             uint8_t plane1, uint8_t plane2);

  /// \brief Match queued up clusters into events.
  ///         Clusters with end times within max_delta_time are joined.
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations
  void match(bool flush) override;

private:
  uint64_t max_delta_time_{0};

  uint64_t delta_end(const Event &event, const Cluster &cluster) const;
  bool belongs_end(const Event &event, const Cluster &cluster) const;
};
