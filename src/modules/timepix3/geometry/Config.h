// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/config/Config.h>
#include <common/debug/Trace.h>

#include <cstdint>
#include <string>
#include <sys/types.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

class Config : public Configurations::Config {

public:
  Config();

  Config(const std::string &ConfigFile);

  // clang-format off
  std::string InstrumentName;
  uint16_t XResolution{0};      /// Resolution along x axis
  uint16_t YResolution{0};      /// Resolution along y axis
  uint8_t ScaleUpFactor{1};     /// Scale up factor for super resolution
  uint16_t ParallelThreads{1};
  float FrequencyHz{14.0};      /// Frequency of the ESS proton pulse
  // clang-format on

  uint32_t MaxTimeGapNS{1};
  uint32_t MinEventSizeHits{1};
  uint32_t MinimumToTSum{20};
  uint32_t MinEventTimeSpanNS{1};
  uint16_t MaxCoordinateGap{5};
};
} // namespace Timepix3
