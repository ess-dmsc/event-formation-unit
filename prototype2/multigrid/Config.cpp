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

  reduction.max_wire_hits = root["max_wire_hits"];
  reduction.max_grid_hits = root["max_grid_hits"];

  reduction.analyzer.weighted(root["weighted"]);
  reduction.analyzer.mappings = mappings;

  // deduced geometry from MG mappings
  reduction.geometry.nx(mappings.max_x());
  reduction.geometry.ny(mappings.max_y());
  reduction.geometry.nz(mappings.max_z());
  reduction.geometry.np(1);
}

std::string Config::debug() const {
  std::stringstream ss;
  if (builder)
    ss << builder->debug();
  else
    ss << "  ========           No Builder :(           ========\n";

  ss << "  Event position using weighted average: "
     << (reduction.analyzer.weighted() ? "YES" : "no") << "\n";

  // \todo multiplicity filter

  ss << "  geometry_x = " << reduction.geometry.nx() << "\n";
  ss << "  geometry_y = " << reduction.geometry.ny() << "\n";
  ss << "  geometry_z = " << reduction.geometry.nz() << "\n";

  return ss.str();
}

}
