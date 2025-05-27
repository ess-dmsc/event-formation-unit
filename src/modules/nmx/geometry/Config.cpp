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
  // Initialize parameters
  setMask(LOG);
  assign("DefaultMinADC", NMXFileParameters.DefaultMinADC);
  assign("SizeX", NMXFileParameters.SizeX);
  assign("SizeY", NMXFileParameters.SizeY);
  assign("MaxSpanX", NMXFileParameters.MaxSpanX);
  assign("MaxSpanY", NMXFileParameters.MaxSpanY);
  assign("MinSpanX", NMXFileParameters.MinSpanX);
  assign("MinSpanY", NMXFileParameters.MinSpanY);
  assign("MaxGapX", NMXFileParameters.MaxGapX);
  assign("MaxGapY", NMXFileParameters.MaxGapY);
  assign("MaxMatchingTimeGap", NMXFileParameters.MaxMatchingTimeGap);
  assign("MaxClusteringTimeGap", NMXFileParameters.MaxClusteringTimeGap);
  assign("NumModules", NMXFileParameters.NumModules);
  assign("SplitMultiEvents", NMXFileParameters.SplitMultiEvents);
  assign("SplitMultiEventsCoefficientLow", NMXFileParameters.SplitMultiEventsCoefficientLow);
  assign("SplitMultiEventsCoefficientHigh", NMXFileParameters.SplitMultiEventsCoefficientHigh);

  try {
    auto ModuleConfig = root()["Config"];
    for (auto &Mapping : ModuleConfig) {
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
        Module[Ring][FEN][LocalHybrid] = Mapping["Module"].get<uint64_t>();
        XTRACE(INIT, DEB, "Got Module: %u", Module[Ring][FEN][LocalHybrid]);
      } catch (...) {
        Module[Ring][FEN][LocalHybrid] = 0;
      }

      Hybrid.MinADC = NMXFileParameters.DefaultMinADC;
    }

    // 2 Hybrids represent the x and y of a single square of 128 * 128 pixels,
    // resulting in 16,384 pixels per 2 hybrids, therefore 8192 pixels per
    // hybrid
    NumPixels += NumHybrids * 8192;

  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Config file: {}",
        configFile());
    throw std::runtime_error("Invalid Json file");
  }
}

} // namespace Nmx
