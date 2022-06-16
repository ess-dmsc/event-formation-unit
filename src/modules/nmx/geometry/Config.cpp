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
    NMXFileParameters.DefaultMinADC = root["DefaultMinADC"].get<std::uint16_t>();
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
    NMXFileParameters.NumPanels = root["NumPanels"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for NumPanels");
  }
  LOG(INIT, Sev::Info, "Size Z {}", NMXFileParameters.NumPanels);

  try {
    auto PanelConfig = root["Config"];
    for (auto &Mapping : PanelConfig) {
      uint8_t Ring = Mapping["Ring"].get<uint8_t>();
      uint8_t FEN = Mapping["FEN"].get<uint8_t>();
      uint8_t LocalHybrid = Mapping["Hybrid"].get<uint8_t>();
      std::string IDString = Mapping["HybridId"];

      XTRACE(INIT, DEB, "Ring %u, FEN %u, Hybrid %u", Ring, FEN, LocalHybrid);

      ESSReadout::Hybrid &Hybrid = getHybrid(Ring, FEN, LocalHybrid);

      
      std::string VesselID = Mapping["VesselId"];
      Plane[Ring][FEN][LocalHybrid] =
          Mapping["Plane"];
     
      try {
        ReversedChannels[Ring][FEN][LocalHybrid] = Mapping["ReversedChannels"];
      } catch (...) {
        ReversedChannels[Ring][FEN][LocalHybrid] = false;
      }

       try {
        Offset[Ring][FEN][LocalHybrid] = Mapping["Offset"];
      } catch (...) {
        Offset[Ring][FEN][LocalHybrid] = 0;
      }

      try {
        Hybrid.MinADC =
            root["Vessel_Config"][VesselID]["MinADC"];
        XTRACE(INIT, DEB, "Vessel specific MinADC %u assigned to vessel %s",
               Hybrid.MinADC, VesselID.c_str());
      } catch (...) {
        Hybrid.MinADC = NMXFileParameters.DefaultMinADC;
      }
    }
  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Config file: {}", FileName);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Nmx
