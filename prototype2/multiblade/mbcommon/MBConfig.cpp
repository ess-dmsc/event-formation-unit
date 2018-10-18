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

namespace Multiblade {

///
MBConfig::MBConfig(std::string jsonfile) : ConfigFile(jsonfile) {

  loadConfigFile();

  if (!isConfigLoaded()) {
    throw std::runtime_error("Unable to load configuration file.");
  }

  if (Instrument == InstrumentGeometry::Estia) {
    Detector = std::make_shared<MB16Detector>(Digitisers);
  } else if (Instrument == InstrumentGeometry::Freia) {
    Detector = std::make_shared<MB16Detector>(Digitisers); /// \todo add parameters for Freia
  }

  assert(Detector != nullptr);
}

///
void MBConfig::loadConfigFile() {
  nlohmann::json root;

  if (ConfigFile.empty()) {
    LOG(INIT, Sev::Info, "JSON config - no config file specified, using default configuration");
    return;
  }

  LOG(INIT, Sev::Info, "JSON config - loading configuration from file {}", ConfigFile);
  std::ifstream t(ConfigFile);
  std::string jsonstring((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());

  try {
    root = nlohmann::json::parse(jsonstring);
  }
  catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}", ConfigFile);
    return;
  }
  /// extract config parameters below

  try {
    auto instr = root["InstrumentGeometry"].get<std::string>();

    if (instr.compare("Estia") == 0) {
      Instrument = InstrumentGeometry::Estia;
    } else if (instr.compare("Freia") == 0) {
      Instrument = InstrumentGeometry::Freia;
    } else {
      LOG(INIT, Sev::Warning, "JSON config - error: Unknown instrument specified, using default (Estia)");
    }
  }
  catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: parser error for InstrumentGeometry");
    return;
  }

  try {
    auto digitisers = root["DigitizerConfig"];
    for (auto &digitiser : digitisers) {
      struct MB16Detector::Digitiser digit;
      digit.index = digitiser["index"].get<unsigned int>();
      digit.digid = digitiser["id"].get<unsigned int>();
      Digitisers.push_back(digit);
      LOG(INIT, Sev::Info, "JSON config - Digitiser {}, offset {}", digit.digid, digit.index);
    }
  }
  catch (...) {
    Digitisers.clear();
    LOG(INIT, Sev::Error, "JSON config error: parser error for DigitizerConfig");
    return;
  }

  try {
    TimeTickNS = root["TimeTickNS"].get<uint32_t>();
    assert(TimeTickNS != 0);
  }
  catch (...) {
    LOG(INIT, Sev::Error, "JSON config error: parser error for TimeTickNS");
    return;
  }

  IsConfigLoaded = true;
}

}
