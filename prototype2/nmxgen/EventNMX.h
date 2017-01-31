/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Classes for NMX event formation
 */
 
#ifndef EVENT_NMX_H
#define EVENT_NMX_H

#include <numeric>
#include <limits>
#include <set>
#include <inttypes.h>
#include <list>

struct EntryNMX
{
  uint64_t time;
  uint8_t  plane_id;
  uint8_t  strip;
  uint16_t adc;
};

struct PlaneNMX
{
  void push(const EntryNMX& e);
  void analyze(bool weighted, uint16_t max_timebins, uint16_t max_timedif);
  
  
  double center;
  int16_t uncert_lower, uncert_upper;

  uint64_t time_start, time_end;
  double integral;
  
  std::list<EntryNMX> entries;
};

struct EventNMX
{
  void push(const EntryNMX& e);
  void analyze(bool weighted, int16_t max_timebins, int16_t max_timedif);
  
  bool good {false};
  PlaneNMX x, y;
};

#endif
