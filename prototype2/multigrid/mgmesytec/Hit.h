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

struct MGHit {

  size_t trigger_count {0};
  int8_t external_trigger {0};
  uint8_t module {0};
  uint32_t high_time {0};
  uint32_t low_time {0};
  uint64_t total_time {0};
  uint8_t bus {0};
  uint16_t channel {0};
  uint16_t adc {0};
  uint16_t time_diff {0};

  // \brief prints values for debug purposes
  std::string debug() const;
};
