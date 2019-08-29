/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/JsonFile.h>
#include <multigrid/Config.h>
#include <sstream>
#include <multigrid/mesytec/BuilderReadouts.h>
#include <multigrid/mesytec/BuilderMesytec.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop
#include <common/Log.h>
#include <common/Trace.h>

namespace Multigrid {

Config::Config(const std::string& jsonfile, std::string dump_path) {
  nlohmann::json root = from_json_file(jsonfile);

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
