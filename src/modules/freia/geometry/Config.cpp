/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/JsonFile.h>
#include <common/Log.h>
#include <freia/geometry/Config.h>

namespace Freia {

///
Config::Config() {}

Config::Config(std::string ConfigFile) {
  nlohmann::json root = from_json_file(ConfigFile);

  try {
    auto Name = root["Detector"].get<std::string>();

    auto PanelConfig = root["Config"];
    unsigned int RingIndex{0};
    unsigned int Cassettes{0};
    for (auto &Mapping : PanelConfig) {
      auto Ring = Mapping["Ring"].get<unsigned int>();
      auto Offset = Mapping["CassOffset"].get<unsigned int>();
      auto FENs = Mapping["FENs"].get<unsigned int>();

      if ((Ring != RingIndex) or (Ring > 10)) {
        LOG(INIT, Sev::Error, "Ring configuration error");
        throw std::runtime_error("Inconsistent Json file - ring index mismatch");
      }

      MaxFen.push_back(FENs);
      Cassettes += FENs * 2;
      RingIndex++;

      LOG(INIT, Sev::Info,
          "JSON config - Detector {}, Ring {}, Offset {}, FENs {}",
          Name, Ring, Offset, FENs);
    }

    Pixels = Cassettes * 32 * 64; //
    LOG(INIT, Sev::Info, "JSON config - Detector has {} cassettes and "
    "{} pixels", Cassettes, Pixels);


  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        ConfigFile);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Freia
