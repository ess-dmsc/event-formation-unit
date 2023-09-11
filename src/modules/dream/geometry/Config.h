// Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file
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

  enum DetectorInstance { NONE, DREAM, MAGIC };
  enum ModuleType { BwEndCap, FwEndCap, Mantle, HR, SANS, FR, PA };
  // clang-format off
  std::map<std::string, ModuleType> ModuleTypeMap = {
      // DREAM Detectors
      {"BwEndCap", BwEndCap},
      {"FwEndCap", FwEndCap},
      {"Mantle", Mantle},
      {"HR", HR},
      {"SANS", SANS},
      // MAGIC Detectors
      {"PADetector", PA},
      {"FRDetector", FR}
    };
  // clang-format on

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
  DetectorInstance Instance{NONE};
  std::string Name; // Name specified in json, used to check later

private:
};
} // namespace Dream
