// Copyright (C) 2019 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <bifrost/geometry/BifrostConfig.h>
#include <common/config/Config.h>
#include <common/debug/Trace.h>
#include <cstdint>
#include <loki/geometry/LokiConfig.h>
#include <string>
#include <sys/types.h>
#include <tbl3he/geometry/Tbl3HeConfig.h>

namespace Caen {

class Config : public Configurations::Config {

public:
  /// \brief default constructor (useful for unit tests)
  Config();

  /// \brief constructor used in EFU to load json from file
  Config(const std::string &ConfigFile);

  /// \brief parse the loaded json object
  virtual void parseConfig();

  struct CaenParameters {
    std::string InstrumentName;
    uint32_t Resolution{1};                  // user for cspec only
    uint32_t MaxTOFNS{20 * 71'428'571};      // < Twenty 14Hz pulses
    uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 times 14 hz pulse period
    uint32_t MaxFEN{0};
    uint32_t MaxGroup{0};
    // Non configurable parameters
    int MinRing{0};  // 0-11
    int MaxRing{11}; // 0-11
  } CaenParms;

  LokiConfig LokiConf;
  Tbl3HeConfig Tbl3HeConf;
  BifrostConfig BifrostConf;
};
} // namespace Caen
