// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
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


class VMM3Config {
public:
 
  static constexpr uint8_t MaxRing{
      10}; // 12 (logical) rings from 0 to 11, 11 reserved for monitors
  static constexpr uint8_t MaxFEN{2};    // This is topology specific
  static constexpr uint8_t MaxHybrid{1}; // Hybrids are VMM >> 1

  VMM3Config(){};

  // Load and apply the json config
  VMM3Config(std::string Instrument, std::string ConfigFile)
      :ExpectedName(Instrument), FileName(ConfigFile) {}

  // load file into json object and apply
  void loadAndApply();

  void loadAndApplyConfig();

  void loadAndApplyCalibration(std::string CalibFile);

  void applyVMM3Calibration(ESSReadout::Hybrid & Hybrid, unsigned vmmid,
        nlohmann::json VMMCalibration);

  // Apply the loaded json file
  virtual void apply() = 0;

  // Get Hybrid from the Ring, FEN, and VMM numbers
  // Currently Hybrids are stored as a 3D array, but may be updated in future
  ESSReadout::Hybrid &getHybrid(uint8_t Ring, uint8_t FEN, uint8_t VMM) {
    return Hybrids[Ring][FEN][VMM];
  }

  ESSReadout::Hybrid &getHybrid(std::string HybridID){
    for(int RingID=0 ; RingID <= MaxRing ; RingID++){
      for(int FENID=0 ; FENID <= MaxFEN ; FENID++){
        for(int HybridNum=0 ; HybridNum <= MaxHybrid ; HybridNum++){
          if (Hybrids[RingID][FENID][HybridNum].HybridId == HybridID ){
            return Hybrids[RingID][FENID][HybridNum];
          }
        }
      }
    }
    XTRACE(INIT, ERR, "HybridID %s requested not in configuration file", HybridID.c_str());
    throw std::runtime_error("Invalid HybridID requested");
  }

  void applyCalibration(std::string HybridID,
  nlohmann::json Calibration);

public:
  // Derived parameters
  ESSReadout::Hybrid Hybrids[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];

  uint8_t NumHybrids{0};
  uint32_t NumPixels{0};

  // Other parameters
  std::string ExpectedName{""};
  std::string FileName{""};
  // JSON object
  nlohmann::json root;
};

