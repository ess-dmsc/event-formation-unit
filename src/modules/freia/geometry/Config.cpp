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
    FreiaFileParameters.MaxGapWire = root["MaxGapWire"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapWire");
  }
  LOG(INIT, Sev::Info, "MaxGapWire {}", FreiaFileParameters.MaxGapWire);

  try {
    FreiaFileParameters.MaxGapStrip = root["MaxGapStrip"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapStrip");
  }
  LOG(INIT, Sev::Info, "MaxGapStrip {}", FreiaFileParameters.MaxGapStrip);

  try {
    FreiaFileParameters.SplitMultiEvents = root["SplitMultiEvents"].get<bool>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for SplitMultiEvents");
  }
  LOG(INIT, Sev::Info, "SplitMultiEvents {}",
      FreiaFileParameters.SplitMultiEvents);

  try {
    FreiaFileParameters.SplitMultiEventsCoefficientLow =
        root["SplitMultiEventsCoefficientLow"].get<float>();
  } catch (...) {
    LOG(INIT, Sev::Info,
        "Using default value for SplitMultiEventsCoefficientLow");
  }
  LOG(INIT, Sev::Info, "SplitMultiEventsCoefficientLow {}",
      FreiaFileParameters.SplitMultiEventsCoefficientLow);

  try {
    FreiaFileParameters.SplitMultiEventsCoefficientHigh =
        root["SplitMultiEventsCoefficientHigh"].get<float>();
  } catch (...) {
    LOG(INIT, Sev::Info,
        "Using default value for SplitMultiEventsCoefficientHigh");
  }
  LOG(INIT, Sev::Info, "SplitMultiEventsCoefficientHigh {}",
      FreiaFileParameters.SplitMultiEventsCoefficientHigh);

  try {
    FreiaFileParameters.MaxMatchingTimeGap =
        root["MaxMatchingTimeGap"].get<float>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxMatchingTimeGap");
  }
  LOG(INIT, Sev::Info, "MaxMatchingTimeGap {}",
      FreiaFileParameters.MaxMatchingTimeGap);

  try {
    FreiaFileParameters.MaxClusteringTimeGap =
        root["MaxClusteringTimeGap"].get<float>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxClusteringTimeGap");
  }
  LOG(INIT, Sev::Info, "MaxClusteringTimeGap {}",
      FreiaFileParameters.MaxClusteringTimeGap);

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
      uint8_t CassetteId{0};

      ESSReadout::Hybrid &Hybrid = getHybrid(Ring, FEN, LocalHybrid);

      /// \todo implement extra rows?
      Hybrid.XOffset = 0;

      try {
        CassetteId = (uint8_t)Mapping["CassetteNumber"];
        Hybrid.YOffset = CassetteId * NumWiresPerCassette;
      } catch (...) {
        Hybrid.YOffset = 0;
      }
      XTRACE(INIT, DEB, "Cass %u (Max cassette %u), Ring %u, FEN %u, Hybrid %u, Yoffset %u",
             CassetteId, MaxCassetteNumber, Ring, FEN, LocalHybrid, Hybrid.YOffset);
    }

    NumPixels = NumHybrids * NumWiresPerCassette * NumStripsPerCassette;
  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Config file: {}",
        FileName);
    throw std::runtime_error("Invalid Json file");
  }
}

} // namespace Freia
