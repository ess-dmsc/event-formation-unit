/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Classes for NMX event formation
 */

#pragma once

#include <gdgem/nmx/Eventlet.h>
#include <limits>
#include <list>
#include <string>

struct PlaneNMX {

  /** @brief adds eventlet to event's plane
   * @param eventlet to be added
   */
  void insert_eventlet(const Eventlet &eventlet);

  /** @brief analyzes particle track
   * @param weighted determine entry strip using weighted average
   * @param max_timebins maximum number of timebins to consider for upper
   * uncertainty
   * @param max_timedif maximum span of timebins to consider for upper
   * uncertainty
   */
  void analyze(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  // @brief prints values for debug purposes
  std::string debug() const;

  // @brief returns calculated and rounded entry strip number for pixid
  int16_t center_rounded() const;

  double center{std::numeric_limits<double>::quiet_NaN()}; // entry strip
  int16_t uncert_lower{
      -1}; // lower uncertainty (strip span of eventlets in latest timebin)
  int16_t uncert_upper{
      -1}; // upper uncertainty (strip span of eventlets in latest few timebins)

  double time_start{0}; // start of event timestamp
  double time_end{0};   // end of event timestamp
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
   * @param max_timebins maximum number of timebins to consider for upper
   * uncertainty
   * @param max_timedif maximum span of timebins to consider for upper
   * uncertainty
   */
  void analyze(bool weighted, int16_t max_timebins, int16_t max_timedif);

  // @brief indicates if entry strips were determined in for both planes
  bool valid() const;

  // @brief indicates if both dimensions meet lower uncertainty criterion
  bool meets_lower_cirterion(int16_t max_lu) const;

  // @brief returns timestamp for start of event (earlier of 2 planes)
  double time_start() const;

  // @brief prints values for debug purposes
  std::string debug() const;

  void debug2();

  PlaneNMX x, y; // tracks in x and y planes

private:
  bool valid_{false};      // event has valid entry strips in both planes
  double time_start_{0}; // start of event timestamp (earlier of 2 planes)
};
