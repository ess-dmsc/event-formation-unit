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
    Parms.TimeBoxNs = root["TimeBoxNs"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for TimeBoxNs");
  }
  LOG(INIT, Sev::Info, "TimeBoxNs {}", Parms.TimeBoxNs);

  try {
    auto PanelConfig = root["Config"];
    for (auto &Mapping : PanelConfig) {
      uint8_t Ring = Mapping["Ring"].get<uint8_t>();
      uint8_t FEN = Mapping["FEN"].get<uint8_t>();
      uint8_t LocalHybrid = Mapping["Hybrid"].get<uint8_t>();
      std::string IDString =  Mapping["HybridId"];

      XTRACE(INIT, DEB, "Ring %d, FEN %d, Hybrid %d", Ring, FEN, LocalHybrid);

      if ((Ring > MaxRing) or (FEN - 1 > MaxFEN) or (LocalHybrid > MaxHybrid)) {
        XTRACE(INIT, ERR, "Illegal Ring/FEN/VMM values");
        throw std::runtime_error("Illegal Ring/FEN/VMM values");
      }

      if (Hybrids[Ring][FEN][LocalHybrid].Initialised){
        XTRACE(INIT, ERR, "Duplicate Hybrid in config file");
        throw std::runtime_error("Duplicate Hybrid in config file");
      }

      Hybrids[Ring][FEN][LocalHybrid].Initialised = true;
      Hybrids[Ring][FEN][LocalHybrid].HybridId = IDString;

      LOG(INIT, Sev::Info,
          "JSON config - Detector {}, Ring {}, FEN {}, LocalHybrid {}",
          Name, Ring, FEN, LocalHybrid);


    }

    //Calculates number of pixels config covers via Vessel_Config and 
    //assumed 6 * 16 wires per column and 2 columns per vessel
    try{
      uint8_t NumGrids = 0;
      for (auto &Vessel : root["Vessel_Config"]){
        NumGrids = Vessel["NumGrids"];
        NumPixels+= NumGrids * 6 * 16 * 2;
      }
    } catch(...){
      LOG(INIT, Sev::Error, "Invalid Vessel_Config");
    }
    

  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Config file: {}",
        FileName);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

uint8_t Config::getNumHybrids() {
    uint8_t NumHybrids = 0;
    for (int RingIndex = 0; RingIndex <= MaxRing; RingIndex++){
      for (int FENIndex = 0; FENIndex <= MaxFEN; FENIndex++){
        for (int HybridIndex = 0; HybridIndex <= MaxHybrid; HybridIndex++){
          ESSReadout::Hybrid Hybrid = getHybrid(RingIndex, FENIndex, HybridIndex);
          if (Hybrid.Initialised) {
            NumHybrids++;
          }
        }
      } 
    }
    return NumHybrids;
  }



} // namespace Cspec
