/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <fstream>
#include <multiblade/mbcommon/MBConfig.h>
#include <nlohmann/json.hpp>

///
MBConfig::MBConfig(std::string jsonfile) : ConfigFile(jsonfile) {

  loadConfigFile();

  if (instrument == InstrumentGeometry::Estia) {
    detector = new MB16Detector();
  }

  assert(detector != nullptr);
}

///
void MBConfig::loadConfigFile() {
  nlohmann::json root;

  if (ConfigFile.empty()) {
    LOG(Sev::Info, "JSON config - no config file specified, using default configuration");
    return;
  }

  LOG(Sev::Info, "JSON config - loading configuration from file {}", ConfigFile);
  std::ifstream t(ConfigFile);
  std::string jsonstring((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  try {
    root = nlohmann::json::parse(jsonstring);
  }
  catch (...) {
    LOG(Sev::Error, "JSON config - error: Invalid Json file: {}", ConfigFile);
    return;
  }
    /// extract config parameters below

  try {
    auto instr = root["InstrumentGeometry"].get<std::string>();

    if (instr.compare("Estia") == 0) {
      instrument = InstrumentGeometry::Estia;
    } else if (instr.compare("Freia") == 0 ) {
      instrument = InstrumentGeometry::Freia;
    } else {
      LOG(Sev::Warning, "JSON config - error: Unknown instrument specified, using default (Estia)");
    }
  }
  catch (...) {
    LOG(Sev::Error, "JSONC config - error: parser error for InstrumentGeometry");
    return;
  }

  try {
    auto digitisers = root["DigitizerConfig"];
    for (auto &digitiser : digitisers) {
      auto index = digitiser["index"].get<unsigned int>();
      auto digid = digitiser["id"].get<unsigned int>();
      LOG(Sev::Info, "JSON config - Digitiser {}, offset {}", digid, index);
    }
  }
  catch (...) {
    LOG(Sev::Error, "JSONC config error: parser error for DigitizerConfig");
    return;
  }

  try {
    TimeTickNS = root["TimeTickNS"].get<uint32_t>();
    assert(TimeTickNS != 0);
  }
  catch (...) {
    LOG(Sev::Error, "JSONC config error: parser error for TimeTickNS");
    return;
  }

  IsValidConfig = true;
}
