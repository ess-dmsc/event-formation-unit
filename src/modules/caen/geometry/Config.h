// Copyright (C) 2019 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <loki/geometry/LokiConfig.h>
#include <tbl3he/geometry/Tbl3HeConfig.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class Config {
public:
  ///\brief default constructor (useful for unit tests)
  Config();

  ///\brief constructor used in EFU to load json from file
  Config(const std::string &ConfigFile);

  ///\brief parse the loaded json object
  void parseConfig();

  // Should be moved to BIFROST config
  struct {
    std::string InstrumentName;
    uint16_t NGroupsTotal{0};        // total number of groups in instrument
    uint16_t Resolution{0};          /// Resolution along straws
    uint32_t ReadoutConstDelayNS{0}; /// added to readout data timestamp
    uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9
    uint32_t MaxTOFNS{800000000};
    uint8_t MaxRing{0};
    uint8_t MaxFEN{0};
    uint8_t MaxGroup{14};
    int MaxAmpl{(std::numeric_limits<int>::max)()};
} Legacy;

  LokiConfig LokiConf;
  Tbl3HeConfig Tbl3HeConf;

  std::string ConfigFileName{""};
  nlohmann::json root; // configuration (json)
};
} // namespace Caen
