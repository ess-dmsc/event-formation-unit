// Copyright (C) 2019 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief use nlohmann::json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <caen/geometry/Config.h>

#include <common/debug/Log.h>
#include <common/debug/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

Config::Config() {}

Config::Config(const std::string &ConfigFile)
  : Configurations::Config(ConfigFile) {
  XTRACE(INIT, DEB, "Loading json file");
  loadFromFile();
  XTRACE(INIT, DEB, "Loaded json file");
}

void Config::parseConfig() {
  setMask(CHECK | XTRACE);
  if (!assign("Detector", Legacy.InstrumentName)) {
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
    LokiConf.setRoot(root());
    LokiConf.parseConfig();
  }

  if (Legacy.InstrumentName == "tbl3he") {
    Tbl3HeConf.setRoot(root());
    Tbl3HeConf.parseConfig();
  }

  if ((Legacy.InstrumentName == "bifrost") or (Legacy.InstrumentName == "miracles")) {
    try {
      // Assumed the same for all straws in all banks
      setMask(LOG | CHECK);
      assign("StrawResolution", Legacy.Resolution);

      setMask(LOG);
      assign("MaxPulseTimeNS", Legacy.MaxPulseTimeNS);
      assign("MaxTOFNS",       Legacy.MaxTOFNS);

      setMask(LOG | XTRACE);
      assign("MaxRing",  Legacy.MaxRing);
      assign("MaxFEN",   Legacy.MaxFEN);
      assign("MaxAmpl",  Legacy.MaxAmpl);
      assign("MaxGroup", Legacy.MaxGroup);
    } catch (...) {
      LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
          configFile());
      throw std::runtime_error("Invalid Json file");
    }
  }
}

} // namespace Caen
