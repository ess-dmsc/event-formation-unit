/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Classes for NMX event formation
 */

#pragma once

#include <NMX/Eventlet.h>
#include <list>

struct PlaneNMX {
  void insert_eventlet(const Eventlet &e);
  void analyze(bool weighted, uint16_t max_timebins, uint16_t max_timedif);

  double center;
  int16_t uncert_lower, uncert_upper;

  uint64_t time_start, time_end;
  double integral;

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
  uint64_t time_start_;
};
