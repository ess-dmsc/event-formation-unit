// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Gets detector configuration from json file,
///  and optionally detector calibration from json file
///
/// Inherits from common/readout/VMM3Config.h
/// Provides TREX specific applyConfig function
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/Hybrid.h>
#include <common/readout/vmm3/VMM3Config.h>

#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Trex {

class Config : public vmm3::VMM3Config {
public:
  Config() { FileParameters.InstrumentGeometry = "TREX"; };

  // Load and apply the json config
  Config(const std::string &Instrument, const std::string &ConfigFile)
      : vmm3::VMM3Config(Instrument, ConfigFile) {}

  // Apply the loaded json file
  void applyConfig() override;

public:
  // Parameters obtained from JSON config file
  struct {
    uint16_t SizeX{384};
    uint16_t SizeY{140};
    uint16_t SizeZ{16};
    uint16_t MaxGridsSpan{3};
    uint16_t DefaultMinADC{50};
    uint16_t MaxMatchingTimeGap{500};
    uint16_t MaxClusteringTimeGap{500};
  } TREXFileParameters;

  // Derived parameters
  // TREX specific Hybrid fields not included in common Hybrid class
  bool Rotated[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
  bool Short[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
};
} // namespace Trex
