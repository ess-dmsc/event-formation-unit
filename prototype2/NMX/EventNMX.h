/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Classes for NMX event formation
 */

#pragma once

#include <limits>
#include <list>
#include <NMX/Eventlet.h>
#include <numeric>
#include <set>

struct PlaneNMX {
  void push(const Eventlet &e);
  void analyze(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  double center;
  int16_t uncert_lower, uncert_upper;

  uint64_t time_start, time_end;
  double integral;

  std::list<Eventlet> entries;
};

struct EventNMX {
  void push(const Eventlet &e);
  void analyze(bool weighted, int16_t max_timebins, int16_t max_timedif);

  bool good{false};
  PlaneNMX x, y;
};
