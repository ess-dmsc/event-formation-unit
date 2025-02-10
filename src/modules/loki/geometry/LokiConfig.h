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
  LokiConfig(const std::string &ConfigFile);

  ///\brief parse the loaded json object
  void parseConfig();


  // New and temporary \todo move somewhere else
  // assumption is that Ring, FEN and LocalGroup have
  // already been validated
  int getGlobalGroup(int Ring, int FEN, int LocalGroup) {
    auto & RParm = Parms.Rings[Ring];
    int FENOffset = RParm.FENOffset;

    int Bank = RParm.Bank;
    auto & BParm = Parms.Banks[Bank];
    int YOffset = BParm.YOffset;
    int GroupsN = BParm.GroupsN;

    int Z = LocalGroup % 4;
    //     Bank pos
    return YOffset/7 + Z * GroupsN + (FENOffset+FEN)*2 + LocalGroup/4;
  }

  // New and temporary \todo move somewhere else
  int getY(int Ring, int FEN, int Group, int Unit) {
    auto & RParm = Parms.Rings[Ring];
    int Bank = RParm.Bank;
    int FENOffset = RParm.FENOffset;

    auto & BParm = Parms.Banks[Bank];
    int UnitsPerLayer = BParm.GroupsN * 7;
    int Layer = Group % 4;
    return BParm.YOffset + Layer * UnitsPerLayer + (FENOffset+FEN)*2*7 + 7*(Group/4) + Unit;
  }


  struct BankCfg {
    int GroupsN{0};
    std::string BankName{""};
    int YOffset{0};
  };

  struct RingCfg {
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

    int ConfiguredBanks{0};
    int ConfiguredRings{0};
    int GroupsZ{0};
    int TotalGroups{0};
    static constexpr int NumBanks{9};
    static constexpr int NumRings{11};
    struct BankCfg Banks[NumBanks];
    struct RingCfg Rings[NumRings];
  } Parms;


  std::string ConfigFileName{""};
  nlohmann::json root; // configuration (json)
};
} // namespace Caen
