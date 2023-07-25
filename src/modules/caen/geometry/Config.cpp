// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <caen/geometry/Config.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

///
Config::Config() {}

Config::Config(std::string ConfigFile) : ConfigFileName(ConfigFile) {
  XTRACE(INIT, DEB, "Loading json file");
  root = from_json_file(ConfigFile);
  XTRACE(INIT, DEB, "Loaded json file");
}

void Config::parseConfig() {
  try {
    InstrumentName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if ((InstrumentName != "loki") and (InstrumentName != "bifrost") and
      (InstrumentName != "miracles") and (InstrumentName != "cspec")) {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error("Inconsistent Json file - invalid name, expected "
                             "loki, bifrost, miracles, or cspec");
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
      MaxGroup = root["MaxGroup"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "MaxGroup: {}", MaxGroup);
    XTRACE(INIT, DEB, "MaxGroup: %u", MaxGroup);

    if (InstrumentName == "loki") {
      auto PanelConfig = root["PanelConfig"];
      for (auto &Mapping : PanelConfig) {
        XTRACE(INIT, DEB, "Loading panel");
        auto Bank = Mapping["Bank"].get<unsigned int>();
        bool Vertical = Mapping["Vertical"].get<bool>();
        auto GroupsZ = Mapping["GroupsZ"].get<unsigned int>();
        auto GroupsN = Mapping["GroupsN"].get<unsigned int>();
        auto UnitOffset = Mapping["StrawOffset"].get<unsigned int>();

        NGroupsTotal += GroupsZ * GroupsN;
        LOG(INIT, Sev::Info, "NGroupsTotal: {}", NGroupsTotal);

        LOG(INIT, Sev::Info,
            "JSON config - Detector {}, Bank {}, Vertical {}, GroupsZ {}, "
            "GroupsN "
            "{}, UnitOffset {}",
            InstrumentName, Bank, Vertical, GroupsZ, GroupsN, UnitOffset);

        XTRACE(INIT, DEB,
               "JSON config - GroupsZ %u, GroupsN %u "
               ", UnitOffset %u",
               GroupsZ, GroupsN, UnitOffset);

        PanelGeometry Temp(GroupsZ, GroupsN, UnitOffset);
        Panels.push_back(Temp);
      }
    }
  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        ConfigFileName);
    throw std::runtime_error("Invalid Json file");
  }
}

} // namespace Caen
