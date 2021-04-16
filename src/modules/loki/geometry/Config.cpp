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

      // Assumed the same for all straws in all banks
      Resolution = root["StrawResolution"].get<unsigned int>();

      try {
        ReadoutConstDelayNS = root["ReadoutConstDelayNS"].get<unsigned int>();
        LOG(INIT, Sev::Info, "ReadoutConstDelayNS: {}", ReadoutConstDelayNS);
      } catch (...) {
        ReadoutConstDelayNS = 0;
      }

      auto PanelConfig = root["PanelConfig"];
      for (auto &Mapping : PanelConfig) {
        auto Ring = Mapping["Bank"].get<unsigned int>();
        bool Vertical = Mapping["Vertical"].get<bool>();
        auto TubesZ = Mapping["TubesZ"].get<unsigned int>();
        auto TubesN = Mapping["TubesN"].get<unsigned int>();
        auto StrawOffset = Mapping["StrawOffset"].get<unsigned int>();

        NTubesTotal += TubesZ * TubesN;
        LOG(INIT, Sev::Info, "NTubesTotal: {}", NTubesTotal);

        LOG(INIT, Sev::Info, "JSON config - Detector {}, Ring {}, Vertical {}, TubesZ {}, TubesN {}, StrawOffset {}",
          Name, Ring, Vertical, TubesZ, TubesN, StrawOffset);

        PanelGeometry Temp(TubesZ, TubesN, StrawOffset);
        Panels.push_back(Temp);
      }

      Pixels = NTubesTotal * PanelGeometry::NStraws * Resolution;
      Geometry = new ESSGeometry(Resolution, NTubesTotal * 7, 1, 1);
      LOG(INIT, Sev::Info, "Total pixels: {}", Pixels);

    }
    catch (...) {
      LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}", ConfigFile);
      throw std::runtime_error("Invalid Json file");
      return;
    }
}

} // namespace
