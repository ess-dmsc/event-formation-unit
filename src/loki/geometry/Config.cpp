/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <loki/geometry/Config.h>
#include <common/JsonFile.h>

namespace Loki {

///
Config::Config() {}

Config::Config(std::string ConfigFile) {
    nlohmann::json root = from_json_file(ConfigFile);

    try {
      auto Name = root["Detector"].get<std::string>();

      auto PanelConfig = root["PanelConfig"];
      uint8_t Index{0};
      for (auto &Mapping : PanelConfig) {
        auto Ring = Mapping["Ring"].get<unsigned int>();
        assert(Ring == Index);
        bool Vertical = Mapping["Vertical"].get<bool>();
        auto TubesZ = Mapping["TubesZ"].get<unsigned int>();
        auto TubesN = Mapping["TubesN"].get<unsigned int>();
        auto Offset = Mapping["Offset"].get<unsigned int>();

        Pixels += TubesZ * TubesN * 7 * 512; ///< \todo not hardcode - should parametrise

        LOG(INIT, Sev::Info, "JSON config - Detector {}, Ring {}, Vertical {}, TubesZ {}, TubesN {}, Offset {}",
          Name, Ring, Vertical, TubesZ, TubesN, Offset);

        PanelGeometry Temp(Vertical, TubesZ, TubesN, Offset);
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
