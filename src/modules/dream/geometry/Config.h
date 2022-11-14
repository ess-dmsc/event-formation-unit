// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
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
#include <map>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {
class Config {
public:
  static constexpr int MaxRing{11};
  static constexpr int MaxFEN{11};

  enum ModuleType { BwEndCap, FwEndCap, Mantle, HR, SANS };
  std::map<std::string, ModuleType> ModuleTypeMap = {{"BwEndCap", BwEndCap},
                                                     {"FwEndCap", FwEndCap},
                                                     {"Mantle", Mantle},
                                                     {"HR", HR},
                                                     {"SANS", SANS}};

  struct ModuleParms {
    bool Initialised{false};
    ModuleType Type;
    union {
      int Index;  // for Cuboids
      int MU;     // for Mantle
      int Sector; // for Endcaps
    } P1;
    union {
      int Index;
      int Cassette; // for Mantle
      int SumoPair; // for endcaps
      int Rotate;   // for Cuboids
    } P2;
  };

  //
  Config(std::string ConfigFile) : FileName(ConfigFile) {
    memset(RMConfig, 0, sizeof(RMConfig));
  };

  //
  Config(){};

  // load file into json object and apply
  void loadAndApply();

  // Apply the loaded json file
  void apply();

  /// \brief log errormessage and throw runtime exception
  void errorExit(std::string errmsg);

  uint32_t MaxPulseTimeDiffNS{5 * 71'428'571}; // 5 * 1/14 * 10^9

  ModuleParms RMConfig[MaxRing + 1][MaxFEN + 1];

  //
  std::string FileName{""};
  nlohmann::json root;

private:
};
} // namespace Dream
