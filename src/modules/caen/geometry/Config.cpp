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

  if (InstrumentName == "loki") {
    LokiConf.root = root;
    LokiConf.parseConfig();
  }

  if ((InstrumentName == "bifrost") or (InstrumentName == "miracles")) {
    try {
      // Assumed the same for all straws in all banks
      Resolution = root["StrawResolution"].get<unsigned int>();

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

    } catch (...) {
      LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
          ConfigFileName);
      throw std::runtime_error("Invalid Json file");
    }
  }
}

} // namespace Caen
