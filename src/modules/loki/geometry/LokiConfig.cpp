// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <loki/geometry/LokiConfig.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

///
LokiConfig::LokiConfig() {}

LokiConfig::LokiConfig(std::string ConfigFile) : ConfigFileName(ConfigFile) {
  XTRACE(INIT, DEB, "Loading json file");
  root = from_json_file(ConfigFile);
  XTRACE(INIT, DEB, "Loaded json file");
}

void LokiConfig::parseConfig() {
  try {
    Parms.InstrumentName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if (Parms.InstrumentName != "loki") {
    LOG(INIT, Sev::Error, "Invalid instrument name ({}) for loki", Parms.InstrumentName);
    throw std::runtime_error("InstrumentName != 'loki'");
  }

  try {
    // Assumed the same for all straws in all banks
    Parms.Resolution = root["Resolution"].get<unsigned int>();

    try {
      Parms.ReadoutConstDelayNS = root["ReadoutConstDelayNS"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "ReadoutConstDelayNS: {}", Parms.ReadoutConstDelayNS);

    try {
      Parms.MaxPulseTimeNS = root["MaxPulseTimeNS"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "MaxPulseTimeNS: {}", Parms.MaxPulseTimeNS);

    try {
      Parms.MaxTOFNS = root["MaxTOFNS"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "MaxTOFNS: {}", Parms.MaxTOFNS);


    // auto PanelConfig = root["PanelConfig"];
    // for (auto &Mapping : PanelConfig) {
    //   XTRACE(INIT, DEB, "Loading panel");
    //   auto Bank = Mapping["Bank"].get<unsigned int>();
    //   bool Vertical = Mapping["Vertical"].get<bool>();
    //   auto GroupsZ = Mapping["GroupsZ"].get<unsigned int>();
    //   auto GroupsN = Mapping["GroupsN"].get<unsigned int>();
    //   auto UnitOffset = Mapping["StrawOffset"].get<unsigned int>();
    //
    //   NGroupsTotal += GroupsZ * GroupsN;
    //   LOG(INIT, Sev::Info, "NGroupsTotal: {}", NGroupsTotal);
    //
    //   LOG(INIT, Sev::Info,
    //       "JSON config - Detector {}, Bank {}, Vertical {}, GroupsZ {}, "
    //       "GroupsN "
    //       "{}, UnitOffset {}",
    //       InstrumentName, Bank, Vertical, GroupsZ, GroupsN, UnitOffset);
    //
    //   XTRACE(INIT, DEB,
    //          "JSON config - GroupsZ %u, GroupsN %u "
    //          ", UnitOffset %u",
    //          GroupsZ, GroupsN, UnitOffset);
    //
    //   PanelGeometry Temp(GroupsZ, GroupsN, UnitOffset);
    //   Panels.push_back(Temp);
    // }

  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        ConfigFileName);
    throw std::runtime_error("Invalid Json file");
  }
}

} // namespace Caen
