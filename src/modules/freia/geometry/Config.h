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
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class Config {
public:
  static constexpr unsigned int NumWiresPerCassette{32};
  static constexpr unsigned int NumStripsPerCassette{64};
  static constexpr uint8_t MaxRing{12};
  static constexpr uint8_t MaxFEN{1};
  static constexpr uint8_t MaxHybrid{1}; // Hybrids are VMM >> 1

  Config() {};

  // Load and apply the json config
  Config(std::string Instrument, std::string ConfigFile)
    : NumFENs(12),
      HybridId(64, -1),
      HybridStr(64),
      InstrumentName(Instrument),
      FileName(ConfigFile) {}

  // load file into json object and apply
  void loadAndApply();

  // Apply the loaded json file
  void apply();


  uint8_t getHybridId(uint8_t Ring, uint8_t FEN, uint8_t VMM) {
    int Id = HybridId[hybridIndex(Ring, FEN, VMM)];
    if (Id < 0) {
      throw std::runtime_error("Unallocated HybridId SNAFU");
    }
    if (Id > 31) {
      throw std::runtime_error("HybridId SNAFU");
    }
    return (uint8_t)Id;
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

  // Configurable parameters
  struct {
    bool StripGapCheck{true};
    bool WireGapCheck{true};
    uint16_t MaxGapWire{0};
    uint16_t MaxGapStrip{0};

    uint32_t MaxPulseTimeNS{5 * 71'428'571}; // 5 * 1/14 * 10^9=
    uint32_t TimeBoxNs{0xffffffff};
  } Parms;

  // Derived parameters
  std::vector<uint16_t> NumFENs;   // #FENs per logical ring
  std::vector<int> HybridId; // reinit in constructor
  std::vector<std::string> HybridStr; // reinit in constructor
  uint8_t NumHybrids{0};
  uint32_t NumPixels{0};
  std::string InstrumentName{""};
  std::string FileName{""};

  // JSON object
  nlohmann::json root;
};

} // namespace Freia
