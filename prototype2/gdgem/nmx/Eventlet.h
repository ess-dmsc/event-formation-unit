/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Eventlet struct for NMX event formation
 */

#pragma once

#include <cinttypes>
#include <limits>
#include <string>

struct Eventlet {
public:
  using strip_type = uint16_t;
  using adc_type = uint16_t;
  static constexpr strip_type strip_max_val{
      std::numeric_limits<strip_type>::max()};
  static constexpr adc_type adc_max_val{std::numeric_limits<adc_type>::max()};

public:
  double time{0};
  uint8_t plane_id{0};
  strip_type strip{0};
  adc_type adc{0};
  bool over_threshold{false};

  // @brief prints values for debug purposes
  std::string debug() const;
};
