/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Classes for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/Event.h>
#include <limits>

namespace Gem
{

struct utpcResultsPlane {
  double utpc_center{std::numeric_limits<double>::quiet_NaN()}; // entry strip
  int16_t uncert_lower{-1}; /// strip span of hits in latest timebin
  int16_t uncert_upper{-1}; /// strip span of hits in latest few timebins

  /// \brief returns calculated and rounded entry strip number for pixid
  uint32_t utpc_center_rounded() const;

  /// \brief prints values for debug purposes
  std::string debug() const;
};

struct utpcResults {
  utpcResultsPlane x, y;
  uint64_t time {0};
  bool good {false};
};

class utpcAnalyzer {
 public:
  /// \param weighted determine entry strip using weighted average
  /// \param max_timebins maximum number of timebins to consider for upper
  /// uncertainty
  /// \param max_timedif maximum span of timebins to consider for upper
  /// uncertainty
  utpcAnalyzer(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  /// \brief analyzes particle track in one plane
  utpcResultsPlane analyze(Cluster&) const;

  /// \brief analyzes particle track in both planes
  utpcResults analyze(Event&) const;

  /// \brief returns timestamp for start of event (earlier of 2 planes)
  static uint64_t utpc_time(const Event& e);

  /// \brief indicates if both dimensions meet lower uncertainty criterion
  static bool meets_lower_criterion(const utpcResultsPlane& x,
                                    const utpcResultsPlane& y, int16_t max_lu);

 private:
  bool weighted_{true};
  uint16_t max_timebins_;
  uint16_t max_timedif_;
};

}
