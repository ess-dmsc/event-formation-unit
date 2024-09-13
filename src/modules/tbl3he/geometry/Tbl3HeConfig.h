// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get tbl3he configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <common/debug/Trace.h>
#include <common/memory/HashMap2D.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class Tbl3HeConfig {
public:
  ///\brief default constructor (useful for unit tests)
  Tbl3HeConfig();

  ///\brief constructor used in EFU to load json from file
  Tbl3HeConfig(std::string ConfigFile);

  ///\brief parse the loaded json object
  void parseConfig();

  struct {
    std::string InstrumentName{""};
    int Resolution{0};
    uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9
    uint32_t MaxTOFNS{800000000};
    int NumRings{12};
    int NumOfFENs{0};
  } Parms;

  struct Topology {
    int Bank{-1};

    Topology() = default;
  };



  std::unique_ptr<HashMap2D<Topology>> TopologyMapPtr;

  std::string ConfigFileName{""};
  nlohmann::json root; // configuration (json)
};
} // namespace Caen
