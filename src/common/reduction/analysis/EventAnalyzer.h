/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Classes for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/analysis/AbstractAnalyzer.h>
#include <limits>

class EventAnalyzer : public AbstractAnalyzer {
 public:
  /// \param time_algorithm determines algorithm to determine position and time
  /// \param max_timebins maximum number of timebins to consider for upper
  /// uncertainty
  /// \param max_timedif maximum span of timebins to consider for upper
  /// uncertainty
  EventAnalyzer(std::string time_algorithm);

  /// \brief analyzes particle track in one plane
  ReducedHit analyze(Cluster&) const;

  /// \brief analyzes particle track in both planes
  ReducedEvent analyze(Event&) const override;

   /// \brief prints info for debug purposes
  std::string debug(const std::string& prepend) const override;
  
  /// \brief indicates if both dimensions meet lower uncertainty criterion
  static bool meets_lower_criterion(const ReducedHit& x,
                                    const ReducedHit& y, int16_t max_lu);

 private:
  std::string time_algorithm_ = "utpc";
};

