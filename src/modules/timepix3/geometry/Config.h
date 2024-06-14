// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {
class Config {
public:
  Config();

  Config(std::string ConfigFile);

  std::string InstrumentName;
  uint16_t XResolution{0}; /// Resolution along x axis
  uint16_t YResolution{0}; /// Resolution along y axis
  uint16_t parallelThreads{1};
  float FrequencyHz{14.0}; /// Frequency of the ESS proton pulse

  uint32_t MaxTimeGapNS{1};
  uint32_t MinEventSizeHits{1};
  uint32_t MinimumToTSum{0};
  uint16_t MaxCoordinateGap{5};
};
} // namespace Timepix3
