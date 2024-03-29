// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <nmx/geometry/Config.h>

#include <iostream>

namespace Nmx {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Config::applyConfig() {
  try {
    NMXFileParameters.DefaultMinADC =
        root["DefaultMinADC"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for DefaultMinADC");
  }
  LOG(INIT, Sev::Info, "DefaultMinADC {}", NMXFileParameters.DefaultMinADC);

  try {
    NMXFileParameters.SizeX = root["SizeX"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for Size X");
  }
  LOG(INIT, Sev::Info, "Size X {}", NMXFileParameters.SizeX);

  try {
    NMXFileParameters.SizeY = root["SizeY"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for Size Y");
  }
  LOG(INIT, Sev::Info, "Size Y {}", NMXFileParameters.SizeY);

  try {
    NMXFileParameters.MaxSpanX = root["MaxSpanX"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxSpanX");
  }
  LOG(INIT, Sev::Info, "MaxSpanX {}", NMXFileParameters.MaxSpanX);

  try {
    NMXFileParameters.MaxSpanY = root["MaxSpanY"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxSpanY");
  }
  LOG(INIT, Sev::Info, "MaxSpanY {}", NMXFileParameters.MaxSpanY);

  try {
    NMXFileParameters.MinSpanX = root["MinSpanX"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MinSpanX");
  }
  LOG(INIT, Sev::Info, "MinSpanX {}", NMXFileParameters.MinSpanX);

  try {
    NMXFileParameters.MinSpanY = root["MinSpanY"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MinSpanY");
  }
  LOG(INIT, Sev::Info, "MinSpanY {}", NMXFileParameters.MinSpanY);

  try {
    NMXFileParameters.MaxGapX = root["MaxGapX"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxGapX");
  }
  LOG(INIT, Sev::Info, "MaxGapX {}", NMXFileParameters.MaxGapX);

  try {
    NMXFileParameters.MaxGapY = root["MaxGapY"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxYGap");
  }
  LOG(INIT, Sev::Info, "MaxGapY {}", NMXFileParameters.MaxGapY);

  try {
    NMXFileParameters.MaxMatchingTimeGap =
        root["MaxMatchingTimeGap"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxMatchingTimeGap");
  }
  LOG(INIT, Sev::Info, "MaxMatchingTimeGap {}",
      NMXFileParameters.MaxMatchingTimeGap);

  try {
    NMXFileParameters.MaxClusteringTimeGap =
        root["MaxClusteringTimeGap"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxClusteringTimeGap");
  }
  LOG(INIT, Sev::Info, "MaxClusteringTimeGap {}",
      NMXFileParameters.MaxClusteringTimeGap);

  try {
    NMXFileParameters.NumPanels = root["NumPanels"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for NumPanels");
  }
  LOG(INIT, Sev::Info, "NumPanels {}", NMXFileParameters.NumPanels);

  try {
    NMXFileParameters.SplitMultiEvents = root["SplitMultiEvents"].get<bool>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for SplitMultiEvents");
  }
  LOG(INIT, Sev::Info, "SplitMultiEvents {}",
      NMXFileParameters.SplitMultiEvents);

  try {
    NMXFileParameters.SplitMultiEventsCoefficientLow =
        root["SplitMultiEventsCoefficientLow"].get<float>();
  } catch (...) {
    LOG(INIT, Sev::Info,
        "Using default value for SplitMultiEventsCoefficientLow");
  }
  LOG(INIT, Sev::Info, "SplitMultiEventsCoefficientLow {}",
      NMXFileParameters.SplitMultiEventsCoefficientLow);

  try {
    NMXFileParameters.SplitMultiEventsCoefficientHigh =
        root["SplitMultiEventsCoefficientHigh"].get<float>();
  } catch (...) {
    LOG(INIT, Sev::Info,
        "Using default value for SplitMultiEventsCoefficientHigh");
  }
  LOG(INIT, Sev::Info, "SplitMultiEventsCoefficientHigh {}",
      NMXFileParameters.SplitMultiEventsCoefficientHigh);

  try {
    auto PanelConfig = root["Config"];
    for (auto &Mapping : PanelConfig) {
      uint8_t Ring = Mapping["Ring"].get<uint8_t>();
      uint8_t FEN = Mapping["FEN"].get<uint8_t>();
      uint8_t LocalHybrid = Mapping["Hybrid"].get<uint8_t>();
      std::string IDString = Mapping["HybridId"];

      XTRACE(INIT, DEB, "Ring %u, FEN %u, Hybrid %u", Ring, FEN, LocalHybrid);

      ESSReadout::Hybrid &Hybrid = getHybrid(Ring, FEN, LocalHybrid);
      XTRACE(INIT, DEB, "Got Hybrid");

      try {
        Plane[Ring][FEN][LocalHybrid] = Mapping["Plane"].get<uint8_t>();
        XTRACE(INIT, DEB, "Got Plane: %u", Plane[Ring][FEN][LocalHybrid]);
      } catch (...) {
        XTRACE(INIT, DEB, "Failed to get Plane, using 0");
        Plane[Ring][FEN][LocalHybrid] = 0;
      }

      try {
        ReversedChannels[Ring][FEN][LocalHybrid] =
            Mapping["ReversedChannels"].get<bool>();
        XTRACE(INIT, DEB, "Got ReversedChannels: %u",
               ReversedChannels[Ring][FEN][LocalHybrid]);
      } catch (...) {
        ReversedChannels[Ring][FEN][LocalHybrid] = false;
      }

      try {
        Offset[Ring][FEN][LocalHybrid] = Mapping["Offset"].get<uint64_t>();
        XTRACE(INIT, DEB, "Got Offset: %u", Offset[Ring][FEN][LocalHybrid]);
      } catch (...) {
        Offset[Ring][FEN][LocalHybrid] = 0;
      }

      try {
        Panel[Ring][FEN][LocalHybrid] = Mapping["Panel"].get<uint64_t>();
        XTRACE(INIT, DEB, "Got Panel: %u", Panel[Ring][FEN][LocalHybrid]);
      } catch (...) {
        Panel[Ring][FEN][LocalHybrid] = 0;
      }

      Hybrid.MinADC = NMXFileParameters.DefaultMinADC;
    }

    // 2 Hybrids represent the x and y of a single square of 128 * 128 pixels,
    // resulting in 16,384 pixels per 2 hybrids, therefore 8192 pixels per
    // hybrid
    NumPixels += NumHybrids * 8192;

  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Config file: {}",
        FileName);
    throw std::runtime_error("Invalid Json file");
  }
}

} // namespace Nmx
