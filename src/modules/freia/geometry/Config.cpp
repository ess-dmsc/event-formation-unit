// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
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

Config::Config(std::string ConfigFile) {
  nlohmann::json root = from_json_file(ConfigFile);

  try {
    auto Name = root["Detector"].get<std::string>();

    auto PanelConfig = root["Config"];
    unsigned int VMMOffs{0};
    unsigned int FENOffs{0};
    for (auto &Mapping : PanelConfig) {
      auto Ring = Mapping["Ring"].get<unsigned int>();
      auto Offset = Mapping["CassOffset"].get<unsigned int>();
      auto FENs = Mapping["FENs"].get<unsigned int>();

      if ((Ring != NumRings) or (Ring > 10)) {
        LOG(INIT, Sev::Error, "Ring configuration error");
        throw std::runtime_error("Inconsistent Json file - ring index mismatch");
      }

      NumFens.push_back(FENs);
      FENOffset.push_back(FENOffs);
      VMMOffset.push_back(VMMOffs);

      VMMOffs += FENs * VMMsPerFEN;
      FENOffs += FENs;
      NumCassettes += FENs * CassettesPerFEN;
      NumRings++;

      LOG(INIT, Sev::Info,
          "JSON config - Detector {}, Ring {}, Offset {}, FENs {}",
          Name, Ring, Offset, FENs);
    }

    NumPixels = NumCassettes * NumWiresPerCassette * NumStripsPerCassette; //
    LOG(INIT, Sev::Info, "JSON config - Detector has {} cassettes and "
    "{} pixels", NumCassettes, NumPixels);


  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        ConfigFile);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Freia