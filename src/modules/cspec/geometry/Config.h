// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Gets detector configuration from json file,
///  and optionally detector calibration from json file
///
/// Inherits from common/readout/VMM3Config.h
/// Provides CSPEC specific applyConfig function
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

namespace Cspec {

class Config : public VMM3Config {
public:

  Config(){
    FileParameters.InstrumentGeometry = "CSPEC";
  };

  // Load and apply the json config
  Config(std::string Instrument, std::string ConfigFile)
     : VMM3Config(Instrument, ConfigFile) {}

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
  } CSPECFileParameters;

  
 

  // Derived parameters
  // CSPEC specific Hybrid fields not included in common Hybrid class
  bool Rotated[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
  bool Short[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];
};
} // namespace Cspec
