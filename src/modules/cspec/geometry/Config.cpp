// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <cspec/geometry/Config.h>

#include <iostream>

namespace Cspec {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Config::applyConfig() {
  try {
    DefaultMinADC = root["DefaultMinADC"].get<std::uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for DefaultMinADC");
  }
  LOG(INIT, Sev::Info, "DefaultMinADC {}", DefaultMinADC);

  try {
    SizeX = root["SizeX"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for Size X");
  }
  LOG(INIT, Sev::Info, "Size X {}", SizeX);

  try {
    SizeY = root["SizeY"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for Size Y");
  }
  LOG(INIT, Sev::Info, "Size Y {}", SizeY);

  try {
    SizeZ = root["SizeZ"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for Size Z");
  }
  LOG(INIT, Sev::Info, "Size Z {}", SizeZ);

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
      Rotated[Ring][FEN][LocalHybrid] =
          root["Vessel_Config"][VesselID]["Rotation"];
     
      try {
        Short[Ring][FEN][LocalHybrid] = root["Vessel_Config"][VesselID]["Short"];
      } catch (...) {
        Short[Ring][FEN][LocalHybrid] = false;
      }

      try {
       Hybrid.XOffset = root["Vessel_Config"][VesselID]["XOffset"];
      } catch (...) {
        Hybrid.XOffset = 0;
      }

      try {
       Hybrid.YOffset = root["Vessel_Config"][VesselID]["YOffset"];
      } catch (...) {
        Hybrid.YOffset = 0;
      }

      try {
        Hybrid.MinADC =
            root["Vessel_Config"][VesselID]["MinADC"];
        XTRACE(INIT, DEB, "Vessel specific MinADC %u assigned to vessel %s",
               Hybrid.MinADC, VesselID.c_str());
      } catch (...) {
        Hybrid.MinADC = DefaultMinADC;
      }
    }

    // Calculates number of pixels config covers via Vessel_Config and
    // assumed 6 * 16 wires per column and 2 columns per vessel
    try {
      uint8_t NumGrids = 0;
      for (auto &Vessel : root["Vessel_Config"]) {
        NumGrids = Vessel["NumGrids"];
        NumPixels += NumGrids * 6 * 16 * 2;
      }
    } catch (...) {
      LOG(INIT, Sev::Error, "Invalid Vessel_Config");
    }

  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Config file: {}", FileName);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Cspec
