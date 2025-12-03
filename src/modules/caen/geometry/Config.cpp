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
  if (!assign("Detector", CaenParms.InstrumentName)) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if ((CaenParms.InstrumentName != "loki") and
      (CaenParms.InstrumentName != "bifrost") and
      (CaenParms.InstrumentName != "miracles") and
      (CaenParms.InstrumentName != "cspec") and
      (CaenParms.InstrumentName != "tbl3he")) {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error("Inconsistent Json file - invalid name, expected "
                             "loki, bifrost, tbl3he, miracles, or cspec");
  }

  try {
    setMask(LOG);
    assign("MaxPulseTimeNS", CaenParms.MaxPulseTimeNS);
    assign("MaxTOFNS", CaenParms.MaxTOFNS);

    setMask(LOG | CHECK);
    assign("Resolution", CaenParms.Resolution);

    setMask(LOG | XTRACE);
    assign("MaxFEN", CaenParms.MaxFEN);
    assign("MaxGroup", CaenParms.MaxGroup);
  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        configFile());
    throw std::runtime_error("Invalid Json file");
  }

  if (CaenParms.InstrumentName == "loki") {
    LokiConf.setRoot(root());
    LokiConf.parseConfig();
  }

  if (CaenParms.InstrumentName == "tbl3he") {
    Tbl3HeConf.setRoot(root());
    Tbl3HeConf.parseConfig();
  }

  if ((CaenParms.InstrumentName == "bifrost") or
      (CaenParms.InstrumentName == "miracles")) {
    BifrostConf.setRoot(root());
    BifrostConf.parseConfig();
  }
}

} // namespace Caen
