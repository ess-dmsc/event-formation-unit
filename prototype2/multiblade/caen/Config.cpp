/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <fstream>
#include <multiblade/caen/Config.h>
#include <nlohmann/json.hpp>

namespace Multiblade {

///
Config::Config(std::string jsonfile) : ConfigFile(jsonfile) {

  loadConfigFile();

  if (!isConfigLoaded()) {
    throw std::runtime_error("Unable to load configuration file.");
  }

  if (Instrument == InstrumentGeometry::Estia) {
    Detector = std::make_shared<DigitizerMapping>(Digitisers);

  } else if (Instrument == InstrumentGeometry::Freia) {
    Detector = std::make_shared<DigitizerMapping>(Digitisers); /// \todo add parameters for Freia
  }

  assert(Detector != nullptr);
}

///
void Config::loadConfigFile() {
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

  /// extract config parameters below

    auto instr = root["InstrumentGeometry"].get<std::string>();

    if (instr.compare("Estia") == 0) {
      Instrument = InstrumentGeometry::Estia;
    } else if (instr.compare("Freia") == 0) {
      Instrument = InstrumentGeometry::Freia;
    } else {
      LOG(INIT, Sev::Error, "JSON config - error: Unknown instrument specified");
      return;
    }

    auto det = root["Detector"].get<std::string>();

    if (det.compare("MB18") == 0) {
      DetectorType = DetectorType::MB18;
    } else if (det.compare("MB16") == 0) {
      DetectorType = DetectorType::MB16;
    } else {
      LOG(INIT, Sev::Warning, "JSON config - error: Unknown detector specified");
      return;
    }

    NCass = root["cassettes"].get<unsigned int>();
    NWires  = root["wires"].get<unsigned int>();
    NStrips = root["strips"].get<unsigned int>();

    if ((NWires == 0) or (NStrips == 0) or (NCass == 0)) {
      LOG(INIT, Sev::Warning, "JSON config - error: Unknown detector specified");
      return;
    }

    auto digitisers = root["DigitizerConfig"];
    for (auto &digitiser : digitisers) {
      struct DigitizerMapping::Digitiser digit;
      digit.index = digitiser["index"].get<unsigned int>();
      digit.digid = digitiser["id"].get<unsigned int>();
      Digitisers.push_back(digit);
      LOG(INIT, Sev::Info, "JSON config - Digitiser {}, offset {}", digit.digid, digit.index);
    }

    TimeTickNS = root["TimeTickNS"].get<uint32_t>();
    assert(TimeTickNS != 0);
  }
  catch (...) {
    LOG(INIT, Sev::Error, "JSON config - error: Invalid Json file: {}", ConfigFile);
    return;
  }

  IsConfigLoaded = true;
}

}
