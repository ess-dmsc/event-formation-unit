// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
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

class Config : public vmm3::VMM3Config {
public:
  Config() { FileParameters.InstrumentGeometry = "NMX"; };

  // Load and apply the json config
  Config(const std::string &Instrument, const std::string &ConfigFile)
      : vmm3::VMM3Config(Instrument, ConfigFile) {}

  // Apply the loaded json file
  void applyConfig() override;

public:
  // Parameters obtained from JSON config file
  struct {
    uint16_t SizeX{1280};
    uint16_t SizeY{1280};
    uint16_t NumPanels{4};
    uint16_t MaxSpanX{10};
    uint16_t MaxSpanY{10};
    uint16_t MinSpanX{1}; //< \todo should be larger?
    uint16_t MinSpanY{1}; //< \todo should be larger?
    uint16_t MaxGapX{10}; //< \todo unused?
    uint16_t MaxGapY{10}; //< \todo unused?
    uint16_t DefaultMinADC{50};
    bool SplitMultiEvents{false};
    float SplitMultiEventsCoefficientLow{0.8};
    float SplitMultiEventsCoefficientHigh{1.2};
    uint16_t MaxTimeSpan{500};
    uint16_t MaxMatchingTimeGap{500};
    uint16_t MaxClusteringTimeGap{500};
  } NMXFileParameters;

  // Derived parameters
  // NMX specific Hybrid fields not included in common Hybrid class
  // Grouped into a single struct for cache-friendly access
  struct HybridParams {
    uint64_t Offset{0};
    uint8_t Plane{0};
    uint8_t Panel{0};
    bool ReversedChannels{false};
  };
  
  // Single 3D array of structs for cache-optimal access pattern
  // All 4 fields fit in a single cache line (~32 bytes for alignment)
  HybridParams HybridParam[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
};
} // namespace Nmx
