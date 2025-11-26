// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get detector configuration from json file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/config/Config.h>

#include <common/JsonFile.h>
#include <common/debug/Trace.h>
#include <common/readout/vmm3/Hybrid.h>

#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace vmm3 {

class VMM3Config : public Configurations::Config {
public:
  // Provide maximum values from all VMM3 detectors
  static constexpr uint8_t MaxRing{
      10}; // 12 (logical) rings from 0 to 11, 11 reserved for monitors
  static constexpr uint8_t MaxFEN{13};    // This is topology specific
  static constexpr uint8_t MaxHybrid{10}; // Hybrids are VMM >> 1

  VMM3Config(){};

  // Load and apply the json config
  VMM3Config(const std::string &Instrument, const std::string &ConfigFile)
    : Configurations::Config(ConfigFile)
    , ExpectedName(Instrument) {
  }

  // load file into json object and apply
  void loadAndApplyConfig();

  /// \brief Loads calibration file and applies to each hybrid based on string
  /// ID matching CalibFile parameter = string path to calibration json file
  void loadAndApplyCalibration(const std::string &CalibFile);

  /// \brief Applies calibration json object to specified VMM on Hybrid
  void applyVMM3Calibration(Hybrid &Hybrid, unsigned vmmid,
                            nlohmann::json VMMCalibration);

  /// \brief Apply VMM3 generic aspects of loaded configuration json file
  void applyVMM3Config();

  /// \brief Apply detector specific aspects of loaded configuration json file
  virtual void applyConfig() = 0;

  bool validHybridId(const std::string &HybridID) const;

  /// \brief Get Hybrid from the Ring, FEN, and VMM numbers
  // Currently Hybrids are stored as a 3D array, but may be updated in future
  Hybrid &getHybrid(uint8_t Ring, uint8_t FEN, uint8_t VMM) {
    return Hybrids[Ring][FEN][VMM];
  }


  /// \brief Return true, if a Hybrid with a given Id exists (this implementation
  /// uses a fast map-based look-up)
  bool lookupHybrid(const std::string &HybridID) const {
    return (HybridMap.find(HybridID) != HybridMap.cend());
  }

  /// \brief Get a Hybrid with a given Hybrid Id; if no Hybrid is located, an
  /// exception is thrown (this implementation uses a fast map-based look-up)
  Hybrid &getHybrid(const std::string &HybridID) {
    // If Hybrid with a given Id is in the map, return it
    const auto iter = HybridMap.find(HybridID);
    if (iter != HybridMap.cend()) {
      return *(iter->second);
    }

    XTRACE(INIT, ERR, "HybridID %s requested not in configuration file",
           HybridID.c_str());
    throw std::runtime_error("Invalid HybridID requested");
  }

  /// \brief Applies calibration to each VMM on Hybrid matching given Hybrid ID
  void applyCalibration(const std::string &HybridID, const nlohmann::json &Calibration);

public:
  struct {
    std::string InstrumentName{""};
    std::string InstrumentGeometry{""};
    uint32_t MaxTOFNS{1'000'000'000};
    uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9=
  } FileParms;

  // Container used for storing a Hybrid with a given (Ring, FEN, Hybrid)
  Hybrid Hybrids[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];

  // Map used for fast access to a Hybrid with a given Id
  std::map<std::string, Hybrid*> HybridMap;

  uint8_t NumHybrids{0};
  uint32_t NumPixels{0};

  // Other parameters
  std::string ExpectedName{""};
};

} // namespace vmm3
