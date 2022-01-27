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




}

} // namespace Freia
