// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <cbm/geometry/Config.h>
#include "CbmTypes.h"

namespace cbm {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Config::errorExit(std::string ErrMsg) {
  LOG(INIT, Sev::Error, ErrMsg);
  throw std::runtime_error(ErrMsg);
}

void Config::loadAndApply() {
  root = from_json_file(FileName);
  apply();
}

void Config::apply() {
  std::string DetectorName;
  try {
    DetectorName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if (DetectorName != "CBM") {
    LOG(INIT, Sev::Error, "Detector name mismatch, expected CBM");
    throw std::runtime_error("Detector name mismatch, expected CBM");
  }

  try {
    Parms.TypeSubType = root["TypeSubType"].get<std::uint8_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for TypeSubType");
  }
  LOG(INIT, Sev::Info, "TypeSubType {}", Parms.TypeSubType);

  try {
    Parms.MaxPulseTimeDiffNS = root["MaxPulseTimeDiffNS"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxPulseTimeDiffNS");
  }
  LOG(INIT, Sev::Info, "MaxPulseTimeDiffNS {}", Parms.MaxPulseTimeDiffNS);

  try {
    Parms.MaxTOFNS = root["MaxTOFNS"].get<std::uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MaxTOFNS");
  }
  LOG(INIT, Sev::Info, "MaxTOFNS {}", Parms.MaxTOFNS);

  try {
    Parms.MonitorRing = root["MonitorRing"].get<std::uint8_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MonitorRing");
  }
  LOG(INIT, Sev::Info, "MonitorRing {}", Parms.MonitorRing);

  try {
    Parms.MonitorFEN = root["MonitorFEN"].get<std::uint8_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MonitorFEN");
  }
  LOG(INIT, Sev::Info, "MonitorFEN {}", Parms.MonitorFEN);

  try {
    Parms.NumberOfMonitors = root["NumberOfMonitors"].get<std::uint8_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for NumberOfMonitors");
  }
  LOG(INIT, Sev::Info, "NumberOfMonitors {}", Parms.NumberOfMonitors);

  try {
    Parms.MonitorOffset = root["MonitorOffset"].get<int>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for MonitorOffset");
  }
  LOG(INIT, Sev::Info, "MonitorOffset {}", Parms.MonitorOffset);

  int Entry{0};
  nlohmann::json Modules;
  Modules = root["Topology"];
  for (auto &Module : Modules) {
    int FEN{MaxFEN + 1};
    int Channel{MaxChannel + 1};
    std::string Type{""};
    int TypeIndex{0};

    try {
      FEN = Module["FEN"].get<int>();
      Type = Module["Type"];
      Channel = Module["Channel"].get<int>();
      TypeIndex = Module["TypeIndex"].get<int>(); 
    } catch (...) {
      std::runtime_error("Malformed 'Config' section (Need FEN, Type, Channel, TypeIndex)");
    }

    // Check for array sizes and dupliacte entries
    if (FEN > MaxFEN) {
      errorExit(fmt::format("Entry: {}, Invalid FEN: {} Max: {}", Entry, FEN,
                            MaxFEN));
    }
    if (RMConfig[FEN][Channel].isConfigured != false) {
      errorExit(fmt::format("Entry: {}, Duplicate entry for FEN {} Channel {}",
                            Entry, FEN, Channel));
    }

    // Now add the relevant parameters
    RMConfig[FEN][Channel].Type = CbmType(Type);
    RMConfig[FEN][Channel].TypeIndex = TypeIndex;
    XTRACE(INIT, ALW, "Entry %02d, FEN %02d, Channel %02d, Type %s", Entry, FEN, Channel, Type.c_str());

    // Final housekeeping
    RMConfig[FEN][Channel].isConfigured = true;
    Entry++;
  }
}

} // namespace cbm
