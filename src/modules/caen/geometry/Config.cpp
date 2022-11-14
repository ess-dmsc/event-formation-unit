/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <caen/geometry/Config.h>
#include <common/JsonFile.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Caen {

///
Config::Config() {}

Config::Config(std::string ConfigFile) {
  XTRACE(INIT, DEB, "Loading json file");
  nlohmann::json root = from_json_file(ConfigFile);
  XTRACE(INIT, DEB, "Loaded json file");

  try {
    InstrumentName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if ((InstrumentName != "loki") and (InstrumentName != "bifrost") and
      (InstrumentName != "miracles")) {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error("Inconsistent Json file - invalid name, expected "
                             "loki, bifrost, or miracles");
  }

  try {
    // Assumed the same for all straws in all banks
    Resolution = root["StrawResolution"].get<unsigned int>();

    try {
      ReadoutConstDelayNS = root["ReadoutConstDelayNS"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "ReadoutConstDelayNS: {}", ReadoutConstDelayNS);

    try {
      MaxPulseTimeNS = root["MaxPulseTimeNS"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "MaxPulseTimeNS: {}", MaxPulseTimeNS);

    try {
      MaxTOFNS = root["MaxTOFNS"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "MaxTOFNS: {}", MaxTOFNS);

    try {
      MaxRing = root["MaxRing"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "MaxRing: {}", MaxRing);
    XTRACE(INIT, DEB, "MaxRing: %u", MaxRing);

    try {
      MaxFEN = root["MaxFEN"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "MaxFEN: {}", MaxFEN);
    XTRACE(INIT, DEB, "MaxFEN: %u", MaxFEN);

    try {
      MaxTube = root["MaxTube"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "MaxTube: {}", MaxTube);
    XTRACE(INIT, DEB, "MaxTube: %u", MaxTube);

    if (InstrumentName == "loki") {
      auto PanelConfig = root["PanelConfig"];
      for (auto &Mapping : PanelConfig) {
        XTRACE(INIT, DEB, "Loading panel");
        auto Bank = Mapping["Bank"].get<unsigned int>();
        bool Vertical = Mapping["Vertical"].get<bool>();
        auto TubesZ = Mapping["TubesZ"].get<unsigned int>();
        auto TubesN = Mapping["TubesN"].get<unsigned int>();
        auto StrawOffset = Mapping["StrawOffset"].get<unsigned int>();

        NTubesTotal += TubesZ * TubesN;
        LOG(INIT, Sev::Info, "NTubesTotal: {}", NTubesTotal);

        LOG(INIT, Sev::Info,
            "JSON config - Detector {}, Bank {}, Vertical {}, TubesZ {}, "
            "TubesN "
            "{}, StrawOffset {}",
            InstrumentName, Bank, Vertical, TubesZ, TubesN, StrawOffset);

        XTRACE(INIT, DEB,
               "JSON config - TubesZ %u, TubesN %u "
               ", StrawOffset %u",
               TubesZ, TubesN, StrawOffset);

        PanelGeometry Temp(TubesZ, TubesN, StrawOffset);
        Panels.push_back(Temp);
      }
    }
  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        ConfigFile);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Caen
