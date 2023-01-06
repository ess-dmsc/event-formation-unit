// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Gets detector configuration from json file,
///  and optionally detector calibration from json file
///
/// Inherits from common/readout/VMM3Config.h
/// Provides NMX specific applyConfig function
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

namespace Nmx {

class Config : public VMM3Config {
public:
  Config() { FileParameters.InstrumentGeometry = "NMX"; };

  // Load and apply the json config
  Config(std::string Instrument, std::string ConfigFile)
      : VMM3Config(Instrument, ConfigFile) {}

  // Apply the loaded json file
  void applyConfig() override;

public:
  // Parameters obtained from JSON config file
  struct {
    uint16_t SizeX{1280};
    uint16_t SizeY{1280};
    uint16_t NumPanels{4};
    uint16_t MaxXSpan{10};
    uint16_t MaxYSpan{10};
    uint16_t MinXSpan{1};
    uint16_t MinYSpan{1};
    uint16_t MaxXGap{10};
    uint16_t MaxYGap{10};
    uint16_t DefaultMinADC{50};
    bool SplitMultiEvents{false};
    float SplitMultiEventsCoefficientLow{0.8};
    float SplitMultiEventsCoefficientHigh{1.2};
    uint16_t MaxTimeSpan{500};
  } NMXFileParameters;

  // Derived parameters
  // NMX specific Hybrid fields not included in common Hybrid class
  bool ReversedChannels[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
  uint8_t Plane[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
  uint8_t Panel[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
  uint64_t Offset[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
};
} // namespace Nmx
