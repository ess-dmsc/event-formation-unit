// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Monitor config
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/memory/HashMap2D.h>
#include <common/readout/ess/Parser.h>
#include <memory>
#include <modules/cbm/CbmTypes.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace cbm {

struct Topology {
  const int FEN{0};
  const int Channel{0};
  const std::string Source{"CBM"};
  const CbmType Type{CbmType::EVENT_0D};
  union {
    int param1{0};
    int pixelOffset;
    int maxTofBin;
  };
  union {
    int param2{0};
    int BinCount;
  };

  Topology() = default;

  Topology(const int &FEN, const int &Channel, const std::string &Source,
           const CbmType &Type, const int &param1)
      : FEN(FEN), Channel(Channel), Source(Source), Type(Type),
        param1(param1){};

  Topology(const int &FEN, const int &Channel, const std::string &Source,
           const CbmType &Type, const int &param1, const int &param2)
      : FEN(FEN), Channel(Channel), Source(Source), Type(Type),
        param1(param1), param2(param2){};
};

class Config {

  void errorExit(const std::string &ErrMsg);

public:
  Config(const std::string &ConfigFile) : FileName(ConfigFile){};

  Config(){};

  // load file into json object and apply
  void loadAndApply();

  // Apply the loaded json file
  void apply();

  // Parameters (eventually) obtained from JSON config file
  struct {
    uint8_t TypeSubType{DetectorType::CBM};
    uint32_t MaxTOFNS{20 * 71'428'571};          // < Twenty 14Hz pulses
    uint32_t MaxPulseTimeDiffNS{5 * 71'428'571}; // < Five 14Hz pulses
    uint8_t MonitorRing{11};     // < Ring number for the monitors
    uint8_t NumberOfMonitors{1}; // < Number of monitor in the config
    uint8_t MaxFENId{10};        // < Maximum FEN ID
    uint8_t NumOfFENs{11};       // < Number of FENs, MaxId + 1
  } Parms;

  std::unique_ptr<HashMap2D<Topology>> TopologyMapPtr;

  std::string FileName{""};
  nlohmann::json root;
};

} // namespace cbm
