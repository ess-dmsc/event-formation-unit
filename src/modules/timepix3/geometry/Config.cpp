// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/JsonFile.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <timepix3/geometry/Config.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

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

  if (InstrumentName != "timepix3") {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error(
        "Inconsistent Json file - invalid name, expected timepix3");
  }

  try {
    XResolution = root["XResolution"].get<uint16_t>();
    YResolution = root["YResolution"].get<uint16_t>();
    parallelThreads = root["ParallelThreads"].get<uint16_t>();

  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        ConfigFile);
    throw std::runtime_error("Invalid Json file");
  }

  try {
    MaxTimeGapNS = root["MaxTimeGapNS"].get<uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Warning, "Using default MaxTimeGapNS");
  }
  try {
    FrequencyHz = root["FrequencyHz"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Warning, "Using default Frequency");
  }

  try {
    MinEventSizeHits = root["MinEventSizeHits"].get<uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Warning, "Using default MinEventSizeHits");
  }

  try {
    MinimumToTSum = root["MinimumToTSum"].get<uint32_t>();
  } catch (...) {
    LOG(INIT, Sev::Warning, "Using default MinimumToTSum");
  }

  try {
    MaxCoordinateGap = root["MaxCoordinateGap"].get<uint16_t>();
  } catch (...) {
    LOG(INIT, Sev::Warning, "Using default MaxCoordinateGap");
  }
}

} // namespace Timepix3
