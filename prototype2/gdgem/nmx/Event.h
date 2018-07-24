/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Classes for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/nmx/Cluster.h>
#include <limits>
#include <list>
#include <string>

class Event {
public:
  Cluster x, y; // tracks in x and y planes

  /** \brief adds hit to event
   * @param hit to be added
   */
  void insert_hit(const Hit &e);

  void merge(Cluster& cluster);

  bool empty() const;


  double time_end() const;
  double time_start() const;
  double time_span() const;
  double time_overlap(const Cluster& other) const;
  bool time_overlap_thresh(const Cluster& other, double thresh) const;


  /** \brief analyzes particle track
   * @param weighted determine entry strip using weighted average
   * @param max_timebins maximum number of timebins to consider for upper
   * uncertainty
   * @param max_timedif maximum span of timebins to consider for upper
   * uncertainty
   */
  void analyze(bool weighted, int16_t max_timebins, int16_t max_timedif);

  // \brief indicates if entry strips were determined in for both planes
  bool valid() const;

  // \brief returns timestamp for start of event (earlier of 2 planes)
  double utpc_time() const;

  // \brief indicates if both dimensions meet lower uncertainty criterion
  bool meets_lower_criterion(int16_t max_lu) const;



  // \brief prints values for debug purposes
  std::string debug() const;
  void debug2();

private:
  bool valid_{false};      // event has valid entry strips in both planes
  double utpc_time_{0}; // start of event timestamp (earlier of 2 planes)
};
