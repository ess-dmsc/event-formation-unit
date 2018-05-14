/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Classes for NMX event formation
 */

#pragma once

#include <gdgem/nmx/Eventlet.h>
#include <limits>
#include <list>

struct Cluster {

  int16_t plane_id {-1};

  /** @brief adds eventlet to event's plane
   * @param eventlet to be added
   */
  void insert_eventlet(const Eventlet &eventlet);

  std::list<Eventlet> entries;
  bool empty() const;

  // calculated as eventlets are added
  uint16_t strip_start{0};
  uint16_t strip_end{0};
  uint16_t strip_span() const;

  double time_start{0};
  double time_end{0};
  double time_span() const;

  double adc_sum{0.0};

  double strip_mass{0.0};   // sum of strip*adc
  double strip_center() const;

  double time_mass{0.0};   // sum of time*adc
  double time_center() const;



  void merge(Cluster& other);
  double time_overlap(const Cluster& other) const;
  bool time_touch(const Cluster& other) const;

  /** @brief analyzes particle track
   * @param weighted determine entry strip using weighted average
   * @param max_timebins maximum number of timebins to consider for upper
   * uncertainty
   * @param max_timedif maximum span of timebins to consider for upper
   * uncertainty
   */
  void analyze(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  // only after analyze
  double utpc_center{std::numeric_limits<double>::quiet_NaN()}; // entry strip
  int16_t uncert_lower{-1}; // strip span of eventlets in latest timebin
  int16_t uncert_upper{-1}; // strip span of eventlets in latest few timebins

  // @brief returns calculated and rounded entry strip number for pixid
  uint32_t utpc_center_rounded() const;


  // @brief prints values for debug purposes
  std::string debug() const;

};
