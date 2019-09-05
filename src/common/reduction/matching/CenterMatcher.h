/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file CenterMatcher.h
/// \brief CenterMatcher class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/matching/AbstractMatcher.h>

/// \class CenterMatcher CenterMatcher.h
/// \brief Matcher implementation that joins clusters into events
///         if the center times of the clusters are close in time.

class CenterMatcher : public AbstractMatcher {
public:
  /// Inherit constructor
  using AbstractMatcher::AbstractMatcher;
  
  /// \brief sets the maximum time gap criterion
  /// \param maximum_time_gap maximum time gap between clusters in different planes
  void set_max_delta_time(uint64_t max_delta_time);
  
   /// \brief sets the time algorithm
  /// \param time_algorithm (center-of-mass, charge2, utpc, utpc-weighted)
  void set_time_algorithm(std::string time_algorithm);
  
  
  /// \brief CenterMatcher constructor
  /// \sa AbstractMatcher
  /// CenterMatcher(uint64_t latency, uint8_t plane1, uint8_t plane2);

  /// \brief Match queued up clusters into events.
  ///         Clusters that overlap in time are joined into events.
  /// clusters separated bu no more than allowed_gap_
  /// \param flush if all queued clusters should be matched regardless of
  ///        latency considerations
  void match(bool flush) override;

  /// \brief print configuration of CenterMatcher
  std::string config(const std::string& prepend) const override;
  
private:
  uint64_t max_delta_time_{0};

  //Algorithm for time calculation, either center-of-mass, charge2, or utpc
  std::string time_algorithm_{"center-of-mass"};

  
};

