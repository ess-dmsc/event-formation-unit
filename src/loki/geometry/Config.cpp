/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <loki/geometry/Config.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop

namespace Loki {

///
Config::Config() {}

Config::Config(std::string ConfigFile) {
    nlohmann::json root;

    if (ConfigFile.empty()) {
      LOG(INIT, Sev::Error, "JSON config - no config file specified.");
      throw std::runtime_error("No config file specified.");
    }

    LOG(INIT, Sev::Info, "JSON config - loading configuration from file {}", ConfigFile);
    std::ifstream t(ConfigFile);
    std::string jsonstring((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());

    if (!t.good()) {
      LOG(INIT, Sev::Error, "Invalid Json file: {}", ConfigFile);
      throw std::runtime_error("LoKI config file error - requested file unavailable.");
    }
    /// \todo Above is copied verbatim from multiblade and should be refactored

    try {
      root = nlohmann::json::parse(jsonstring);

      auto Name = root["Detector"].get<std::string>();

      auto PanelConfig = root["PanelConfig"];
      uint8_t Index{0};
      for (auto &Mapping : PanelConfig) {
        auto Ring = Mapping["Ring"].get<unsigned int>();
        assert(Ring == Index);
        bool Vertical = Mapping["Vertical"].get<bool>();
        auto TZ = Mapping["TubesZ"].get<unsigned int>();
        auto TN = Mapping["TubesN"].get<unsigned int>();
        auto Offset = Mapping["Offset"].get<unsigned int>();

        LOG(INIT, Sev::Info, "JSON config - Detector {}, Ring {}, Vertical {}, TubesZ {}, TubesN {}, Offset {}",
          Name, Ring, Vertical, TZ, TN, Offset);

        PanelGeometry Temp(Vertical, TZ, TN, Offset);
        Panels.push_back(Temp);
        Index++;
      }

    }
    catch (...) {
      LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}", ConfigFile);
      throw std::runtime_error("Invalid Json file");
      return;
    }
}

} // namespace
