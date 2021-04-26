// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Trace.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {
class Config {
public:
  Config();

  Config(std::string ConfigFile);

  uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9

private:
};
} // namespace Dream
