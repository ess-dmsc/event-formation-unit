// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
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
  json_check_keys("Config error", root(), RootFields);

  // Initialize and check Version parameter
  setMask(LOG);
  assign("Version", MBFileParameters.Version);
  if (MBFileParameters.Version != 1) {
    auto Msg =
        fmt::format("Invalid config version {} - expected 1", MBFileParameters.Version);
    LOG(INIT, Sev::Error, Msg.c_str());
    throw std::runtime_error(Msg);
  }
  LOG(INIT, Sev::Info, "Config file version {}", MBFileParameters.Version);

  // Initialize all other parameter
  assign("MaxGapWire", MBFileParameters.MaxGapWire);
  assign("MaxGapStrip", MBFileParameters.MaxGapStrip);
  assign("SplitMultiEvents", MBFileParameters.SplitMultiEvents);
  assign("SplitMultiEventsCoefficientLow", MBFileParameters.SplitMultiEventsCoefficientLow);
  assign("SplitMultiEventsCoefficientHigh", MBFileParameters.SplitMultiEventsCoefficientHigh);
  assign("MaxMatchingTimeGap", MBFileParameters.MaxMatchingTimeGap);
  assign("MaxClusteringTimeGap", MBFileParameters.MaxClusteringTimeGap);

  /// RING/FEN/Hybrid
  auto PanelConfig = root()["Config"];

  for (const auto &Mapping : PanelConfig) {
    json_check_keys("Config error", Mapping, MappingFields);

    uint8_t Ring = Mapping["Ring"].get<uint8_t>();
    uint8_t FEN = Mapping["FEN"].get<uint8_t>();
    uint8_t LocalHybrid = Mapping["Hybrid"].get<uint8_t>();

    ESSReadout::Hybrid &Hybrid = getHybrid(Ring, FEN, LocalHybrid);

    uint8_t CassetteId = Mapping["CassetteNumber"].get<uint8_t>();

    if (root()["InstrumentGeometry"] != "Estia") {
    // FREIA + AMOR
      Hybrid.XOffset = 0;
      Hybrid.YOffset = CassetteId * NumWiresPerCassette;
    } else {
      // ESTIA
      Hybrid.YOffset =
          static_cast<uint16_t>(CassetteId / 48) * NumStripsPerCassette;
      Hybrid.XOffset =
          static_cast<uint16_t>(CassetteId % 48) * NumWiresPerCassette;
    }
    XTRACE(INIT, DEB, "Cass %u, Ring %u, FEN %u, Hybrid %u, Xoffset %u, Yoffset %u",
           CassetteId, Ring, FEN, LocalHybrid, Hybrid.XOffset, Hybrid.YOffset);

    /// Thresholds
    /// Version 1: one threshold for asic0 and asic1 at index, 0 and 1
    if (MBFileParameters.Version == 1) {
     XTRACE(INIT, ALW, "Thresholds for VMM3 Hybrid %d at RING/FEN %d/%d",
            LocalHybrid, Ring, FEN);
     auto &Thresholds = Mapping["Thresholds"];

     if ((Thresholds[0].size()) != 1 or (Thresholds[1].size() != 1)) {
       auto ErrMsg =
           fmt::format("version 1 - invalid threshold array size ({}, {})",
                       Thresholds[0].size(), Thresholds[1].size());
       LOG(INIT, Sev::Error, ErrMsg.c_str());
       throw std::runtime_error(ErrMsg);
     }
     Hybrid.ADCThresholds[0].push_back(Thresholds[0][0].get<int>());
     Hybrid.ADCThresholds[1].push_back(Thresholds[1][0].get<int>());
    }
    /// End Thresholds
  }

  NumPixels = NumHybrids * NumWiresPerCassette * NumStripsPerCassette;

  XTRACE(INIT, ALW, "Configuration file loaded successfully");
}

} // namespace Freia
