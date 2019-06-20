/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file GapMatcher.h
/// \brief GapMatcher class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/AbstractMatcher.h>

/// \class GapMatcher GapMatcher.h
/// \brief Matcher implementation that joins clusters into events
///         if they overlap in time.

class GapMatcher : public AbstractMatcher {
public:
  // Inherit constructor
  using AbstractMatcher::AbstractMatcher;

  // \todo document this
  void set_minimum_time_gap(uint64_t minimum_time_gap);
  
  /// \brief GapMatcher constructor
  /// \sa AbstractMatcher
  /// GapMatcher(uint64_t latency, uint8_t plane1, uint8_t plane2);

  /// \brief Match queued up clusters into events.
  ///         Clusters that overlap in time are joined into events.
  /// clusters separated bu no more than allowed_gap_
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations
  void match(bool flush) override;
private:
  uint64_t minimum_time_gap_{0};
};
