/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Classes for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/Hit.h>
#include <common/clustering/Cluster.h>
#include <limits>
#include <list>
#include <vector>

namespace Gem {

struct utpcResults {
  double utpc_center{std::numeric_limits<double>::quiet_NaN()}; // entry strip
  int16_t uncert_lower{-1}; /// strip span of hits in latest timebin
  int16_t uncert_upper{-1}; /// strip span of hits in latest few timebins

  /// \brief returns calculated and rounded entry strip number for pixid
  uint32_t utpc_center_rounded() const;

  /// \brief prints values for debug purposes
  std::string debug() const;
};

class utpcAnalyzer {
 public:
  utpcAnalyzer(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  /// \brief analyzes particle track
  /// \param weighted determine entry strip using weighted average
  /// \param max_timebins maximum number of timebins to consider for upper
  /// uncertainty
  /// \param max_timedif maximum span of timebins to consider for upper
  /// uncertainty
  utpcResults analyze(Cluster&) const;

  /// \brief returns timestamp for start of event (earlier of 2 planes)
  static uint64_t utpc_time(const Cluster& x, const Cluster& y);

  /// \brief indicates if both dimensions meet lower uncertainty criterion
  static bool meets_lower_criterion(const utpcResults& x, const utpcResults& y,
      int16_t max_lu);

 private:
  bool weighted_{true};
  uint16_t max_timebins_;
  uint16_t max_timedif_;
};

}
