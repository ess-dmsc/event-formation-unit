/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/JsonFile.h>
#include <multigrid/Config.h>
#include <sstream>
#include <multigrid/mesytec/BuilderReadouts.h>
#include <multigrid/mesytec/BuilderMesytec.h>

namespace Multigrid {

Config::Config(const std::string& jsonfile, std::string dump_path) {
  nlohmann::json root = from_json_file(jsonfile);
//  try {
//    std::ifstream t(jsonfile);
//    std::string str((std::istreambuf_iterator<char>(t)),
//                    std::istreambuf_iterator<char>());
//
//    if (!t.good()) {
//      XTRACE(INIT, ERR, "Invalid Json file: %s", jsonfile.c_str());
//      throw std::runtime_error("Multigrid config file error - requested file unavailable.");
//    }
//
//    root = nlohmann::json::parse(str);
//  }
//  catch (...) {
//    LOG(INIT, Sev::Warning, "Invalid Json file: {}", jsonfile);
//    return;
//  }

  if (root.count("mappings")) {
    mappings = root["mappings"];
  }

  auto br = root["builder"];
  if (br["type"] == "mesytec") {
    builder = std::make_shared<BuilderMesytec>(mappings, br["spoof_high_time"], dump_path);
  } else if (br["type"] == "readouts") {
    builder = std::make_shared<BuilderReadouts>(mappings, dump_path);
  }

  if (root.count("pipeline_config"))
    reduction = root["pipeline_config"];
}

std::string Config::debug() const {
  std::stringstream ss;
  if (builder)
    ss << builder->debug();
  else
    ss << "  ========           No Builder :(           ========\n";

  ss << "  " << reduction.config("  ");
  return ss.str();
}

}
