// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief use nlohmann::json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <bifrost/geometry/BifrostConfig.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

BifrostConfig::BifrostConfig() {}

void BifrostConfig::parseConfig() {

  std::string InstrumentName;

  try {
    // Get detector/instrument name
    setMask(CHECK | XTRACE);
    assign("Detector", InstrumentName);
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if (InstrumentName != "bifrost" && InstrumentName != "miracles") {
    LOG(INIT, Sev::Error, "Invalid instrument name ({}) for bifrost or miracles",
        InstrumentName);
    throw std::runtime_error("InstrumentName != 'bifrost' && InstrumentName != 'miracles'");
  }

  try {
    setMask(LOG);
    assign("MaxAmpl", Parms.MaxAmpl);
    
  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        configFile());
    throw std::runtime_error("Invalid Json file");
  }
}

} // namespace Caen
