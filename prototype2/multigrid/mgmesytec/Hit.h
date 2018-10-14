/// Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Hit struct for Multigrid event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>

namespace Multigrid {

struct __attribute__ ((packed)) Hit {

  size_t trigger_count{0};
  int8_t external_trigger{0};
  uint8_t module{0};
  uint32_t high_time{0};
  uint32_t low_time{0};
  uint64_t total_time{0};
  uint8_t bus{0};
  uint16_t channel{0};
  uint16_t adc{0};
  uint16_t time_diff{0};

  // \brief prints values for debug purposes
  std::string debug() const;

  // \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "mgmesytec_hits"; }
  static std::string FormatVersion() { return "1.0.0"; }
};

}
