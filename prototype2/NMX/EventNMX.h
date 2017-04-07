/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Classes for NMX event formation
 */

#pragma once

#include <NMX/Eventlet.h>
#include <limits>
#include <list>

struct PlaneNMX {

  /** @brief adds eventlet to event's plane
   * @param eventlet to be added
   */
  void insert_eventlet(const Eventlet &eventlet);

  /** @brief analyzes particle track
   * @param weighted determine entry strip using weighted average
   * @param max_timebins maximum number of timebins to consider for upper uncertainty
   * @param max_timedif maximum span of timebins to consider for upper uncertainty
   */
  void analyze(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  double center{std::numeric_limits<double>::quiet_NaN()}; //entry strip
  int16_t uncert_lower{-1}; // lower uncertainty (strip span of eventlets in latest timebin)
  int16_t uncert_upper{-1}; // upper uncertainty (strip span of eventlets in latest few timebins)

  uint64_t time_start{0}; // start of event timestamp
  uint64_t time_end{0};   // end of event timestamp
  double integral{0.0};   // sum of adc values

  std::list<Eventlet> entries; // eventlets in plane
};

class EventNMX {
public:
  /** @brief adds eventlet to event
   * @param eventlet to be added
   */
  void insert_eventlet(const Eventlet &e);

  /** @brief analyzes particle track
   * @param weighted determine entry strip using weighted average
   * @param max_timebins maximum number of timebins to consider for upper uncertainty
   * @param max_timedif maximum span of timebins to consider for upper uncertainty
   */
  void analyze(bool weighted, int16_t max_timebins, int16_t max_timedif);

  /** @brief indicates if entry strips were determined in for both planes
   */
  bool good() const;

  /** @brief returns timestamp for start of event (earlier of 2 planes)
   */
  uint64_t time_start() const;

  PlaneNMX x, y; // tracks in x and y planes

private:
  bool good_{false}; // event has valid entry strips in both planes
  uint64_t time_start_{0}; // start of event timestamp (earlier of 2 planes)
};
