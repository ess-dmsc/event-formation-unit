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

namespace Freia {

class Config {
public:
  static constexpr unsigned int NumWiresPerCassette{32};
  static constexpr unsigned int NumStripsPerCassette{64};
  static constexpr uint8_t MaxRing{
      10}; // 12 (logical) rings from 0 to 11, 11 reserved for monitors
  static constexpr uint8_t MaxFEN{2};    // This is topology specific
  static constexpr uint8_t MaxHybrid{1}; // Hybrids are VMM >> 1

  Config(){};

  // Load and apply the json config
  Config(std::string Instrument, std::string ConfigFile)
      : NumFENs(12), ExpectedName(Instrument), FileName(ConfigFile) {}

  // load file into json object and apply
  void loadAndApply();

  // Apply the loaded json file
  void apply();

  // Get Hybrid from the Ring, FEN, and VMM numbers
  // Currently Hybrids are stored as a 3D array, but may be updated in future
  ESSReadout::Hybrid &getHybrid(uint8_t Ring, uint8_t FEN, uint8_t VMM) {
    return Hybrids[Ring][FEN][VMM];
  }

private:
  /// \brief return the Hybrid index calculated from Ring, FEN, VMM
  /// Assume - for Freia - that
  /// \param Ring RingId (4 bits)
  /// \param FEN FEN id (1 bit)
  /// \param VMM  (1 bit)
  uint8_t hybridIndex(uint8_t Ring, uint8_t FEN, uint8_t VMM) {
    return ((Ring << 2) + (FEN << 1) + VMM) & 0x3F;
  }

public:
  // Parameters obtained from JSON config file
  struct {
    std::string InstrumentName{""};
    std::string InstrumentGeometry{"Freia"};

    bool StripGapCheck{true};
    bool WireGapCheck{true};
    uint16_t MaxGapWire{0};
    uint16_t MaxGapStrip{0};

    uint32_t MaxTOFNS{1'000'000'000};
    uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9=
    uint32_t TimeBoxNs{0xffffffff};
  } Parms;

  // Derived parameters
  std::vector<uint16_t> NumFENs; // #FENs per logical ring
  ESSReadout::Hybrid Hybrids[MaxRing + 1][MaxFEN + 1][MaxHybrid + 1];

  uint8_t NumHybrids{0};
  uint32_t NumPixels{0};

  // Other parameters
  std::string ExpectedName{""};
  std::string FileName{""};
  // JSON object
  nlohmann::json root;
};

} // namespace Freia
