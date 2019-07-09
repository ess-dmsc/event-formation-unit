/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/Config.h>
#include <fstream>
#include <sstream>
#include <multigrid/mesytec/BuilderReadouts.h>
#include <multigrid/mesytec/BuilderMesytec.h>
#include <nlohmann/json.hpp>
#include <common/Log.h>
#include <common/Trace.h>

namespace Multigrid {

Config::Config(std::string jsonfile, std::string dump_path) {
  nlohmann::json root;
  try {
    std::ifstream t(jsonfile);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());

    if (!t.good()) {
      XTRACE(INIT, ERR, "Invalid Json file: %s", jsonfile.c_str());
      throw std::runtime_error("Multi-Grid config file error - requested file unavailable.");
    }

    root = nlohmann::json::parse(str);
  }
  catch (...) {
    LOG(INIT, Sev::Warning, "Invalid Json file: {}", jsonfile);
    return;
  }

  DigitalGeometry mappings{root["mappings"]};

  auto br = root["builder"];
  if (br["type"] == "mesytec") {
    builder = std::make_shared<BuilderMesytec>(mappings.mapping(),
                                               br["spoof_high_time"],
                                               dump_path);
  } else if (br["type"] == "readouts") {
    builder = std::make_shared<BuilderReadouts>(mappings.mapping(), dump_path);
  }

  ModulePipeline pipeline;

  pipeline.max_wire_hits = root["max_wire_hits"];
  pipeline.max_grid_hits = root["max_grid_hits"];

  pipeline.analyzer.weighted(root["weighted"]);
  pipeline.analyzer.mappings = mappings;

  // deduced geometry from MG mappings
  pipeline.geometry.nx(mappings.max_x());
  pipeline.geometry.ny(mappings.max_y());
  pipeline.geometry.nz(mappings.max_z());
  pipeline.geometry.np(1);

  reduction.pipelines.resize(pipeline.analyzer.mappings.geometry().buses.size(),
                             pipeline);

  for (size_t i = 0; i < reduction.pipelines.size(); ++i) {
    reduction.pipelines[i].matcher = GapMatcher(sequoia_maximum_latency,
                                                i, i + 1);
  }
}

std::string Config::debug() const {
  std::stringstream ss;
  if (builder)
    ss << builder->debug();
  else
    ss << "  ========           No Builder :(           ========\n";

  for (size_t i = 0; i < reduction.pipelines.size(); ++i) {
    ss << "  Module[" << i << "]\n";
    ss << reduction.pipelines[i].debug("  ");
    ss << "\n";
  }
  return ss.str();
}

}
