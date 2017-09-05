/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Eventlet struct for NMX event formation
 */

#pragma once

#include <cinttypes>
#include <string>

struct Eventlet {
  uint64_t time{0};
  uint16_t plane_id{0};
  uint16_t strip{0};
  uint16_t adc{0};
  bool flag{false};
  bool over_threshold{false};

  // @brief prints values for debug purposes
  std::string debug() const;
};
