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

struct ResultsPlane {
  double coord_{std::numeric_limits<double>::quiet_NaN()}; // entry strip
  double time_{std::numeric_limits<double>::quiet_NaN()}; // entry strip
  int16_t uncert_lower_{-1}; /// strip span of hits in latest timebin
  int16_t uncert_upper_{-1}; /// strip span of hits in latest few timebins

  /// \brief returns calculated and rounded entry strip number for pixid
  uint32_t coord_rounded() const;

  /// \brief prints values for debug purposes
  std::string debug() const;
};

struct Results {
  ResultsPlane x_, y_;
  uint64_t time_ {0};
  bool good_ {false};

  std::string debug() const;
};

class EventAnalyzer {
 public:
  /// \param time_algorithm determines algorithm to determine position and time
  /// \param max_timebins maximum number of timebins to consider for upper
  /// uncertainty
  /// \param max_timedif maximum span of timebins to consider for upper
  /// uncertainty
  EventAnalyzer(std::string time_algorithm);

  /// \brief analyzes particle track in one plane
  ResultsPlane analyze(Cluster&) const;

  /// \brief analyzes particle track in both planes
  Results analyze(Event&) const;

  /// \brief indicates if both dimensions meet lower uncertainty criterion
  static bool meets_lower_criterion(const ResultsPlane& x,
                                    const ResultsPlane& y, int16_t max_lu);

 private:
  std::string time_algorithm_ = "utpc";
};

}
