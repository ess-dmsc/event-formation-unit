/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
//===----------------------------------------------------------------------===//
///
/// \file utpcAnalyzer.h
/// \brief utpcAnalyzer class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/analysis/AbstractAnalyzer.h>

/// \class utpcAnalyzer utpcAnalyzer.h
/// \brief utpcAnalyzer Implements a micro-Time Projection Chamber (uTPC)
///        strategy for the analysis of gas electron multiplier (GEM)
///        type of detectors. In general this means picking some "latest"
///        hits for each dimension, and averaging these. It also calculates
///        two uncertainties for this average. Produces 2-dimensional events.

// \todo Make reference to Doro paper

class utpcAnalyzer : public AbstractAnalyzer {
public:
  /// \param weighted determine entry strip using weighted average
  /// \param max_timebins maximum number of time-bins to consider for upper
  /// uncertainty
  /// \param max_timedif maximum span of time-bins to consider for upper
  /// uncertainty
  utpcAnalyzer(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  /// \brief analyzes particle track in one plane
  ReducedHit analyze(Cluster &) const;

  /// \brief analyzes particle track in both planes
  ReducedEvent analyze(Event &) const override;

  /// \brief prints info for debug purposes
  std::string debug(const std::string &prepend) const override;

  /// \brief returns timestamp for start of event (earlier of 2 planes)
  static uint64_t utpc_time(const Event &e);

  // \todo refactor: move this out to some filter class
  /// \brief indicates if both dimensions meet lower uncertainty criterion
  static bool meets_lower_criterion(const ReducedHit &x, const ReducedHit &y,
                                    int16_t max_lu);

private:
  bool weighted_{true};
  uint16_t max_timebins_;
  uint16_t max_timedif_;
};
