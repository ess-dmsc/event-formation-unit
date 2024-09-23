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

  // wrapper function for runtime exception
  void errorExit(std::string ErrMsg);


  ///\brief parse the loaded json object
  void parseConfig();


  struct {
    // configurable parameters
    std::string InstrumentName{""};
    uint32_t MaxPulseTimeNS{0};
    uint32_t MaxTOFNS{0};
    int Resolution{0};
    int NumOfFENs{0};
    int MaxGroup{0};
    int MinValidAmplitude{0};
    // Non configurable parameters
    int MinRing{0};
    int MaxRing{11};
  } Parms;


  struct Topology {
    int Bank{-1};

    Topology(int Bank) : Bank(Bank) {}

    Topology() = default;
  };

  std::unique_ptr<HashMap2D<Topology>> TopologyMapPtr;

  std::string ConfigFileName{""};
  nlohmann::json root; // configuration (json)
};
} // namespace Caen
