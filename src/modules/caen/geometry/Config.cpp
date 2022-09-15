/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/JsonFile.h>
#include <common/debug/Log.h>
#include <caen/geometry/Config.h>

namespace Loki {

///
Config::Config() {}

Config::Config(std::string ConfigFile) {
  nlohmann::json root = from_json_file(ConfigFile);

  std::string InstrumentName;
  try {
    InstrumentName = root["Detector"].get<std::string>();
  } catch (...) {
    LOG(INIT, Sev::Error, "Missing 'Detector' field");
    throw std::runtime_error("Missing 'Detector' field");
  }

  if (InstrumentName != "LoKI") {
    LOG(INIT, Sev::Error, "InstrumentName mismatch");
    throw std::runtime_error(
        "Inconsistent Json file - invalid name, expected LoKI");
  }

  try {
    // Assumed the same for all straws in all banks
    Resolution = root["StrawResolution"].get<unsigned int>();

    try {
      ReadoutConstDelayNS = root["ReadoutConstDelayNS"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "ReadoutConstDelayNS: {}", ReadoutConstDelayNS);

    try {
      MaxPulseTimeNS = root["MaxPulseTimeNS"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "MaxPulseTimeNS: {}", MaxPulseTimeNS);

    try {
      MaxTOFNS = root["MaxTOFNS"].get<unsigned int>();
    } catch (...) {
      // Use default value
    }
    LOG(INIT, Sev::Info, "MaxTOFNS: {}", MaxTOFNS);

    auto PanelConfig = root["PanelConfig"];
    for (auto &Mapping : PanelConfig) {
      auto Bank = Mapping["Bank"].get<unsigned int>();
      bool Vertical = Mapping["Vertical"].get<bool>();
      auto TubesZ = Mapping["TubesZ"].get<unsigned int>();
      auto TubesN = Mapping["TubesN"].get<unsigned int>();
      auto StrawOffset = Mapping["StrawOffset"].get<unsigned int>();

      NTubesTotal += TubesZ * TubesN;
      LOG(INIT, Sev::Info, "NTubesTotal: {}", NTubesTotal);

      LOG(INIT, Sev::Info,
          "JSON config - Detector {}, Bank {}, Vertical {}, TubesZ {}, TubesN "
          "{}, StrawOffset {}",
          InstrumentName, Bank, Vertical, TubesZ, TubesN, StrawOffset);

      PanelGeometry Temp(TubesZ, TubesN, StrawOffset);
      Panels.push_back(Temp);
    }

    Pixels = NTubesTotal * PanelGeometry::NStraws * Resolution;
    // This detector is made of individual 2D banks, so final 2 dimensions are 1
    Geometry =
        new ESSGeometry(Resolution, NTubesTotal * PanelGeometry::NStraws, 1, 1);
    LOG(INIT, Sev::Info, "Total pixels: {}", Pixels);

  } catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}",
        ConfigFile);
    throw std::runtime_error("Invalid Json file");
    return;
  }
}

} // namespace Loki
