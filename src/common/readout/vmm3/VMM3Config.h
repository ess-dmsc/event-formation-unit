// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
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
  // Provide maximum values from all VMM3 detectors
  static constexpr uint8_t MaxRing{
      10}; // 12 (logical) rings from 0 to 11, 11 reserved for monitors
  static constexpr uint8_t MaxFEN{13};    // This is topology specific
  static constexpr uint8_t MaxHybrid{10}; // Hybrids are VMM >> 1

  VMM3Config(){};

  // Load and apply the json config
  VMM3Config(const std::string &Instrument, const std::string &ConfigFile)
      : ExpectedName(Instrument), FileName(ConfigFile) {}

  // load file into json object and apply
  void loadAndApplyConfig();

  /// \brief Loads calibration file and applies to each hybrid based on string
  /// ID matching CalibFile parameter = string path to calibration json file
  void loadAndApplyCalibration(const std::string &CalibFile);

  /// \brief Applies calibration json object to specified VMM on Hybrid
  void applyVMM3Calibration(ESSReadout::Hybrid &Hybrid, unsigned vmmid,
                            nlohmann::json VMMCalibration);

  /// \brief Apply VMM3 generic aspects of loaded configuration json file
  void applyVMM3Config();

  /// \brief Apply detector specific aspects of loaded configuration json file
  virtual void applyConfig() = 0;

  bool validHybridId(const std::string &HybridID);

  /// \brief Get Hybrid from the Ring, FEN, and VMM numbers
  // Currently Hybrids are stored as a 3D array, but may be updated in future
  ESSReadout::Hybrid &getHybrid(uint8_t Ring, uint8_t FEN, uint8_t VMM) {
    return Hybrids[Ring][FEN][VMM];
  }


  /// \brief Get Hybrid from the string Hybrid ID
  /// \todo candidate to refactor as it is nearly the same as getHybrid()
  /// check if HybridId is already configured.
  bool lookupHybrid(const std::string &HybridID) {
    for (int RingID = 0; RingID <= MaxRing; RingID++) {
      for (int FENID = 0; FENID <= MaxFEN; FENID++) {
        for (int HybridNum = 0; HybridNum <= MaxHybrid; HybridNum++) {
          if (Hybrids[RingID][FENID][HybridNum].HybridId == HybridID) {
            return true;
          }
        }
      }
    }
    return false;
  }

  /// \brief Get Hybrid from the string Hybrid ID
  // Slow string comparison method, only to be used on EFU config initialisation
  ESSReadout::Hybrid &getHybrid(const std::string &HybridID) {
    for (int RingID = 0; RingID <= MaxRing; RingID++) {
      for (int FENID = 0; FENID <= MaxFEN; FENID++) {
        for (int HybridNum = 0; HybridNum <= MaxHybrid; HybridNum++) {
          if (Hybrids[RingID][FENID][HybridNum].HybridId == HybridID) {
            return Hybrids[RingID][FENID][HybridNum];
          }
        }
      }
    }
    XTRACE(INIT, ERR, "HybridID %s requested not in configuration file",
           HybridID.c_str());
    throw std::runtime_error("Invalid HybridID requested");
  }

  /// \brief Applies calibration to each VMM on Hybrid matching given Hybrid ID
  void applyCalibration(const std::string &HybridID, nlohmann::json Calibration);

public:
  struct {
    std::string InstrumentName{""};
    std::string InstrumentGeometry{""};
    uint32_t MaxTOFNS{1'000'000'000};
    uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9=
  } FileParameters;

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
