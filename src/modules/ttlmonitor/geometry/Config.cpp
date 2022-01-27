// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <ttlmonitor/geometry/Config.h>

namespace TTLMonitor {

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

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

  if (DetectorName != "TTLMonitor") {
    LOG(INIT, Sev::Error, "InstrumentName mismatch, expexted TTLMonitor");
    throw std::runtime_error("Inconsistent Json file - invalid name, expected TTLMonitor");
  }

  try {
    Parms.TypeSubType = root["TypeSubType"].get<std::uint8_t>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for TypeSubType");
  }
  LOG(INIT, Sev::Info, "TypeSubType {}", Parms.TypeSubType);

  try {
    Parms.TTLMonitorTopic = root["TTLMonitorTopic"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Info, "Using default value for Kafka Topic");
  }
  LOG(INIT, Sev::Info, "TTLMonitorTopic {}", Parms.TTLMonitorTopic);



}

} // namespace Freia
