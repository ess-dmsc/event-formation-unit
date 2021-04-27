// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/JsonFile.h>
#include <common/Log.h>
#include <dream/geometry/Config.h>

namespace Dream {

///
Config::Config() {}

Config::Config(std::string ConfigFile) {
  nlohmann::json root = from_json_file(ConfigFile);

  try {
    auto Name = root["Detector"].get<std::string>();

    try {
      MaxPulseTimeNS = root["MaxPulseTimeNS"].get<unsigned int>();
    } catch (...) { // Use default value
      LOG(INIT, Sev::Info, "MaxPulseTimeNS: {}", MaxPulseTimeNS);
    }

  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        ConfigFile);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Dream
