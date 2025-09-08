// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get loki configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/config/Config.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

class BifrostConfig : public Configurations::Config {
public:
  ///\brief default constructor (useful for unit tests)
  BifrostConfig();

  ///\brief constructor used in EFU to load json from file
  BifrostConfig(const std::string &ConfigFile);

  ///\brief parse the loaded json object
  void parseConfig();

  struct BifrostCfg {
    int MaxAmpl{std::numeric_limits<int>::max()};
  } Parms;
};

} // namespace Caen
