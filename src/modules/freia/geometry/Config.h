/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
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

namespace Freia {
class Config {
public:
  Config();

  Config(std::string ConfigFile);

  std::vector<uint8_t> MaxFen;
  uint32_t Pixels{0};

private:

};
} // namespace Freia
