/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Classes for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/analysis/AbstractAnalyzer.h>

namespace Gem
{

class utpcAnalyzer : public AbstractAnalyzer {
 public:
  /// \param weighted determine entry strip using weighted average
  /// \param max_timebins maximum number of timebins to consider for upper
  /// uncertainty
  /// \param max_timedif maximum span of timebins to consider for upper
  /// uncertainty
  utpcAnalyzer(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  /// \brief analyzes particle track in one plane
  CoordResult analyze(Cluster&) const;

  /// \brief analyzes particle track in both planes
  MultiDimResult analyze(Event&) const override;

  std::string debug() const override;

  /// \brief returns timestamp for start of event (earlier of 2 planes)
  static uint64_t utpc_time(const Event& e);

  /// \brief indicates if both dimensions meet lower uncertainty criterion
  static bool meets_lower_criterion(const CoordResult& x,
                                    const CoordResult& y, int16_t max_lu);

 private:
  bool weighted_{true};
  uint16_t max_timebins_;
  uint16_t max_timedif_;
};

}
