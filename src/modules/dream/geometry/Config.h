// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/config/Config.h>
#include <common/debug/Trace.h>
#include <common/JsonFile.h>

#include <map>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {
class Config : public Configurations::Config {

public:
  Config(const std::string &ConfigFile)
    : Configurations::Config(ConfigFile) {
    memset(RMConfig, 0, sizeof(RMConfig));
  };

  Config(){};

  static constexpr int MaxRing{11};
  static constexpr int MaxFEN{11};

  enum DetectorInstance { NONE, DREAM, MAGIC, HEIMDAL };
  enum ModuleType { BwEndCap, FwEndCap, DreamMantle, HR, SANS, FR, PA, HeimdalMantle};
  // clang-format off
  std::map<std::string, ModuleType> ModuleTypeMap = {
      // DREAM Detectors
      {"BwEndCap", BwEndCap},
      {"FwEndCap", FwEndCap},
      {"DreamMantle", DreamMantle}, // also Magic
      {"HR", HR},
      {"SANS", SANS},
      // MAGIC Detectors
      {"PADetector", PA},
      {"FRDetector", FR},
      // HEIMDAL Detector
      {"HeimdalMantle", HeimdalMantle}
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

  // load file into json object and apply
  void loadAndApply();

  // Apply the loaded json file
  void apply();

  /// \brief log errormessage and throw runtime exception
  void errorExit(const std::string &errmsg);

  uint32_t MaxPulseTimeDiffNS{5 * 71'428'571}; // 5 * 1/14 * 10^9

  ModuleParms RMConfig[MaxRing + 1][MaxFEN + 1];

  DetectorInstance Instance{NONE};

  std::string DetectorName; // Name specified in json, used to check later

private:
};
} // namespace Dream
