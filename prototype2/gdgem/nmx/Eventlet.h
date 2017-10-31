/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Eventlet struct for NMX event formation
 */

#pragma once

#include <cinttypes>
#include <string>
#include <limits>

#define NMX_STRIP_TYPE uint16_t
#define NMX_STRIP_MAX_VAL std::numeric_limits<NMX_STRIP_TYPE>::max()

#define NMX_ADC_TYPE uint16_t
#define NMX_ADC_MAX_VAL std::numeric_limits<NMX_ADC_TYPE>::max()

struct Eventlet {
  uint64_t time{0};
  uint8_t  plane_id{0};
  NMX_STRIP_TYPE strip{0};
  NMX_ADC_TYPE   adc{0};
  bool flag{false};
  bool over_threshold{false};

  // @brief prints values for debug purposes
  std::string debug() const;
};
