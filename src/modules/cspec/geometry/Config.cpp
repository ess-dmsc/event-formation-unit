// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//
#include <iostream>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <cspec/geometry/Config.h>

namespace Cspec {

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

void Config::loadAndApply() {
  root = from_json_file(FileName);
  for ( auto it: root.items() )
  {
    std::cout << it.key() << " | " << it.value() << "\n";
  }
  apply();
}

void Config::apply() {
  std::string Name;
  try {
    Parms.InstrumentName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if (Parms.InstrumentName != ExpectedName) {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error("Inconsistent Json file - invalid name");
  }

  try {
    Parms.InstrumentGeometry = root["InstrumentGeometry"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for InstrumentGeometry");
  }
  LOG(INIT, Sev::Info, "InstrumentGeometry {}", Parms.InstrumentGeometry);

  try {
    Parms.MaxPulseTimeNS = root["MaxPulseTimeNS"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxPulseTimeNS");
  }
  LOG(INIT, Sev::Info, "MaxPulseTimeNS {}", Parms.MaxPulseTimeNS);

  try {
    Parms.MaxGapWire = root["MaxGapWire"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapWire");
  }
  LOG(INIT, Sev::Info, "MaxGapWire {}", Parms.MaxGapWire);

  try {
    Parms.MaxGapStrip = root["MaxGapStrip"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapStrip");
  }
  LOG(INIT, Sev::Info, "MaxGapStrip {}", Parms.MaxGapStrip);

  try {
    Parms.TimeBoxNs = root["TimeBoxNs"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for TimeBoxNs");
  }
  LOG(INIT, Sev::Info, "TimeBoxNs {}", Parms.TimeBoxNs);

  try {
    uint8_t OldRing{255};
    uint8_t OldFEN{255};
    auto PanelConfig = root["Config"];
    for (auto &Mapping : PanelConfig) {
      uint8_t Ring = Mapping["Ring"].get<uint8_t>();
      uint8_t FEN = Mapping["FEN"].get<uint8_t>();
      uint8_t LocalHybrid = Mapping["Hybrid"].get<uint8_t>();
      std::string IDString =  Mapping["HybridId"];

      if (Ring != OldRing) { // New ring
        OldRing = Ring;
        OldFEN = 255;
      } else { // Same ring
        if (FEN != OldFEN) {
          OldFEN = FEN;
          NumFENs[Ring]++;
        }
      }

      XTRACE(INIT, DEB, "Ring %d, FEN %d, Hybrid %d", Ring, FEN, LocalHybrid);

      if ((Ring > MaxRing) or (FEN - 1 > MaxFEN) or (LocalHybrid > MaxHybrid)) {
        XTRACE(INIT, ERR, "Illegal Ring/FEN/VMM values");
        throw std::runtime_error("Illegal Ring/FEN/VMM values");
      }

      // uint8_t HybridIndex = hybridIndex(Ring, FEN - 1, LocalHybrid);

      // if (HybridId[HybridIndex] != -1) {
      //   XTRACE(INIT, ERR, "Duplicate {Ring, FEN, VMM} entry for Hybrid Index %u",
      //     HybridIndex);
      //   throw std::runtime_error("Duplicate {Ring, FEN, VMM} entry");
      // }

      LOG(INIT, Sev::Info,
          "JSON config - Detector {}, Hybrid {}, Ring {}, FEN {}, LocalHybrid {}",
          Name, NumHybrids, Ring, FEN, LocalHybrid);

      // HybridId[HybridIndex] = NumHybrids;
      HybridStr[NumHybrids] = IDString;
      NumHybrids++;
    }

    HybridStr.resize(NumHybrids);

    // NumPixels = NumHybrids * NumWiresPerCassette * NumStripsPerCassette; //
    // LOG(INIT, Sev::Info, "JSON config - Detector has {} cassettes/hybrids and "
    // "{} pixels", NumHybrids, NumPixels);

    for (uint8_t Ring = 0; Ring < 12; Ring++) {
      LOG(INIT, Sev::Info,
          "Ring {} - # FENs {}", Ring, NumFENs[Ring]);
    }


  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Config file: {}",
        FileName);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Cspec
