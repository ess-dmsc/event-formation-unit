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
  void insert_eventlet(const Eventlet &e);
  void analyze(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  double center{std::numeric_limits<double>::quiet_NaN()};
  int16_t uncert_lower{-1};
  int16_t uncert_upper{-1};

  uint64_t time_start{0};
  uint16_t time_end{0};
  double integral{0.0};

  std::list<Eventlet> entries;
};

class EventNMX {
public:
  void insert_eventlet(const Eventlet &e);
  void analyze(bool weighted, int16_t max_timebins, int16_t max_timedif);

  bool good() const;
  uint64_t time_start() const;

  PlaneNMX x, y;

private:
  bool good_{false};
  uint64_t time_start_{0};
};
