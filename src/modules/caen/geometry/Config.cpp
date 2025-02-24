// Copyright (C) 2019 - 2024 European Spallation Source, ERIC. See LICENSE file
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

Config::Config(const std::string &ConfigFile) : ConfigFileName(ConfigFile) {
  XTRACE(INIT, DEB, "Loading json file");
  root = from_json_file(ConfigFile);
  XTRACE(INIT, DEB, "Loaded json file");
}

void Config::parseConfig() {
  try {
    Legacy.InstrumentName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if ((Legacy.InstrumentName != "loki") and (Legacy.InstrumentName != "bifrost") and
      (Legacy.InstrumentName != "miracles") and (Legacy.InstrumentName != "cspec") and
      (Legacy.InstrumentName != "tbl3he")) {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error("Inconsistent Json file - invalid name, expected "
                             "loki, bifrost, tbl3he, miracles, or cspec");
  }

  if (Legacy.InstrumentName == "loki") {
    LokiConf.root = root;
    LokiConf.parseConfig();
  }

  if (Legacy.InstrumentName == "tbl3he") {
    Tbl3HeConf.root = root;
    Tbl3HeConf.parseConfig();
  }

  if ((Legacy.InstrumentName == "bifrost") or (Legacy.InstrumentName == "miracles")) {
    try {
      // Assumed the same for all straws in all banks
      Legacy.Resolution = root["StrawResolution"].get<unsigned int>();

      try {
        Legacy.MaxPulseTimeNS = root["MaxPulseTimeNS"].get<unsigned int>();
      } catch (...) {
        // Use default value
      }
      LOG(INIT, Sev::Info, "MaxPulseTimeNS: {}", Legacy.MaxPulseTimeNS);

      try {
        Legacy.MaxTOFNS = root["MaxTOFNS"].get<unsigned int>();
      } catch (...) {
        // Use default value
      }
      LOG(INIT, Sev::Info, "MaxTOFNS: {}", Legacy.MaxTOFNS);

      try {
        Legacy.MaxRing = root["MaxRing"].get<unsigned int>();
      } catch (...) {
        // Use default value
      }

      LOG(INIT, Sev::Info, "MaxRing: {}", Legacy.MaxRing);
      XTRACE(INIT, DEB, "MaxRing: %u", Legacy.MaxRing);

      try {
        Legacy.MaxFEN = root["MaxFEN"].get<unsigned int>();
      } catch (...) {
        // Use default value
      }
      LOG(INIT, Sev::Info, "MaxFEN: {}", Legacy.MaxFEN);
      XTRACE(INIT, DEB, "MaxFEN: %u", Legacy.MaxFEN);

      try {
        Legacy.MaxAmpl = root["MaxAmpl"].get<int>();
      } catch (...) {
        // Use default (maximum for integer type) value
      }
      LOG(INIT, Sev::Info, "MaxAmpl: {}", Legacy.MaxAmpl);
      XTRACE(INIT, DEB, "MaxAmpl: %u", Legacy.MaxAmpl);

      try {
        Legacy.MaxGroup = root["MaxGroup"].get<unsigned int>();
      } catch (...) {
        // Use default value
      }
      LOG(INIT, Sev::Info, "MaxGroup: {}", Legacy.MaxGroup);
      XTRACE(INIT, DEB, "MaxGroup: %u", Legacy.MaxGroup);

    } catch (...) {
      LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
          ConfigFileName);
      throw std::runtime_error("Invalid Json file");
    }
  }
}

} // namespace Caen
