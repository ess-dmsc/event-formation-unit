// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
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
#include <common/readout/vmm3/Hybrid.h>

#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Cspec {

class Config {
 public:
  static constexpr uint8_t MaxRing{11};   // 12 (logical) rings from 0 to 11
  static constexpr uint8_t MaxFEN{13};    // This is topology specific
  static constexpr uint8_t MaxHybrid{2};  // Hybrids are VMM >> 1

  Config(){};

  // Load and apply the json config
  Config(std::string Instrument, std::string ConfigFile)
      : ExpectedName(Instrument), FileName(ConfigFile) {}

  // load file into json object and apply
  void loadAndApply();

  // Apply the loaded json file
  void apply();

  // Get Hybrid from the Ring, FEN, and VMM numbers
  // Currently Hybrids are stored as a 3D array, but may be updated in future
  ESSReadout::Hybrid& getHybrid(uint8_t Ring, uint8_t FEN, uint8_t VMM) {
    return Hybrids[Ring][FEN][VMM];
  }

  uint8_t getNumHybrids();

 public:
  // Parameters obtained from JSON config file
  struct {
    std::string InstrumentName{""};
    std::string InstrumentGeometry{"CSPEC"};
    uint32_t MaxTOFNS{1'000'000'000};
    uint32_t MaxPulseTimeNS{5 * 71'428'571};  // 5 * 1/14 * 10^9=
    uint32_t TimeBoxNs{0xffffffff};
    uint16_t SizeX = 384;
    uint16_t SizeY = 140;
    uint16_t SizeZ = 16;
    uint16_t MaxGridsSpan = 3;
    uint16_t DefaultMinADC = 50;
  } Parms;

  uint32_t NumPixels{0};

  // Other parameters
  std::string ExpectedName{""};
  std::string FileName{""};
  // JSON object
  nlohmann::json root;

  // Derived parameters
  ESSReadout::Hybrid Hybrids[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
  bool Rotated[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
  bool Short[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
  uint16_t XOffset[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
  uint16_t YOffset[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
  uint16_t MinADC[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
};
}  // namespace Cspec
