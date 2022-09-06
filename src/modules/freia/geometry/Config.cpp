// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <freia/geometry/Config.h>

namespace Freia {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Config::applyConfig() {
  try {
    MaxGapWire = root["MaxGapWire"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapWire");
  }
  LOG(INIT, Sev::Info, "MaxGapWire {}", MaxGapWire);

  try {
    MaxGapStrip = root["MaxGapStrip"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapStrip");
  }
  LOG(INIT, Sev::Info, "MaxGapStrip {}", MaxGapStrip);

  try {
    auto PanelConfig = root["Config"];
    uint8_t MaxCassetteNumber = 0;
    for (auto &Mapping : PanelConfig) {
      if ((uint8_t)Mapping["CassetteNumber"] > MaxCassetteNumber) {
        MaxCassetteNumber = (uint8_t)Mapping["CassetteNumber"];
      }
    }
    for (auto &Mapping : PanelConfig) {
      uint8_t Ring = Mapping["Ring"].get<uint8_t>();
      uint8_t FEN = Mapping["FEN"].get<uint8_t>();
      uint8_t LocalHybrid = Mapping["Hybrid"].get<uint8_t>();
      std::string IDString = Mapping["HybridId"];

      ESSReadout::Hybrid &Hybrid = getHybrid(Ring, FEN, LocalHybrid);

      /// \todo implement extra rows?
      Hybrid.XOffset = 0;

      try {
        Hybrid.YOffset = (MaxCassetteNumber - (uint8_t)Mapping["CassetteNumber"]) *
                         NumWiresPerCassette;
      } catch (...) {
        Hybrid.YOffset = 0;
      }
      XTRACE(INIT, DEB, "MaxCass %u, Ring %u, FEN %u, Hybrid %u, Yoffset %u",
             MaxCassetteNumber, Ring, FEN, LocalHybrid, Hybrid.YOffset);
    }

    NumPixels = NumHybrids * NumWiresPerCassette * NumStripsPerCassette;
  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Config file: {}",
        FileName);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Freia
