// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get loki configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <common/debug/Trace.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class LokiConfig {
public:
  ///\brief default constructor (useful for unit tests)
  LokiConfig();

  ///\brief constructor used in EFU to load json from file
  LokiConfig(std::string ConfigFile);

  ///\brief parse the loaded json object
  void parseConfig();


  struct BankCfg {
    int Groups{0};
    std::string BankName{""};
    int YOffset{0};
  };

  struct RingCfg {
    int Ring{-1};
    int Bank{-1};
    int FENs{0};
    int FENOffset{0};
  };

  struct LokiCfg {
    std::string InstrumentName{""};
    int Resolution{0};
    uint32_t ReadoutConstDelayNS{0}; /// added to readout data timestamp
    uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9
    uint32_t MaxTOFNS{800000000};

    int GroupsZ{4};
    static constexpr int NumBanks{9};
    static constexpr int NumRings{11};
    struct BankCfg Banks[NumBanks];
    struct RingCfg Rings[NumRings];
  } Parms;


  std::string ConfigFileName{""};
  nlohmann::json root; // configuration (json)
};
} // namespace Caen
