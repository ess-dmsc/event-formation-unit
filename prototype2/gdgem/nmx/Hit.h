/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Hit struct for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <limits>
#include <string>

/// \todo rename this to Hit
struct Hit {
  using strip_type = uint16_t;
  using adc_type = uint16_t;
  static constexpr strip_type strip_max_val{std::numeric_limits<strip_type>::max()};
  static constexpr adc_type adc_max_val{std::numeric_limits<adc_type>::max()};

  double time{0};
  uint8_t plane_id{0};
  strip_type strip{0};
  adc_type adc{0};

  // \brief prints values for debug purposes
  std::string debug() const;
};
