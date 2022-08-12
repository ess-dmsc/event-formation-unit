// Copyright (C) 2017-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multi-Grid json configuration: select builder, specify mappings, etc.
///
//===----------------------------------------------------------------------===//

#include <common/JsonFile.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <multigrid/Config.h>
#include <multigrid/mesytec/BuilderMesytec.h>
#include <multigrid/mesytec/BuilderReadouts.h>
#include <sstream>

namespace Multigrid {

Config::Config(const std::string &jsonfile, std::string dump_path) {
  nlohmann::json root = from_json_file(jsonfile);

  if (root.count("mappings")) {
    mappings = root["mappings"];
  }

  auto br = root["builder"];
  if (br["type"] == "mesytec") {
    builder = std::make_shared<BuilderMesytec>(mappings, br["spoof_high_time"],
                                               dump_path);
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

} // namespace Multigrid
