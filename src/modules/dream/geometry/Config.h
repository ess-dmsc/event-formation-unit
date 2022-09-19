// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

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

  enum ModuleType {BwEndCap, FwEndCap, Mantle, HR, SANS};
  std::map<std::string, ModuleType> ModuleTypeMap = {
    {"BwEndCap", BwEndCap},
    {"FwEndCap", FwEndCap},
    {"Mantle",   Mantle},
    {"HR",       HR},
    {"SANS",     SANS}
  };

  struct ModuleParms {
    bool Initialized{false};
    ModuleType Type;
    bool WireIsZ;
    int Wires;
    int Strips;
    int Rotation;
  };

  Config();

  Config(std::string ConfigFile);

  /// \brief log errormessage and throw runtime exception
  void errorExit(std::string errmsg);

  uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9

  ModuleParms RMConfig[MaxRing + 1][MaxFEN + 1];

private:
};
} // namespace Dream
