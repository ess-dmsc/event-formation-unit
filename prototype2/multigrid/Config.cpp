/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/Config.h>
#include <fstream>
#include <sstream>
#include <multigrid/mesytec/BuilderReadouts.h>
#include <multigrid/mesytec/BuilderMesytec.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wc++17-extensions"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop
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

  SequoiaGeometry mappings{root["mappings"]};

  auto br = root["builder"];
  if (br["type"] == "mesytec") {
    builder = std::make_shared<BuilderMesytec>(mappings,
                                               br["spoof_high_time"],
                                               dump_path);
  } else if (br["type"] == "readouts") {
    builder = std::make_shared<BuilderReadouts>(mappings, dump_path);
  }

  max_wire_hits = root["max_wire_hits"];
  max_grid_hits = root["max_grid_hits"];

  analyzer.weighted(root["weighted"]);
  analyzer.mappings = mappings;

  // deduced geometry from MG mappings
  geometry.nx(mappings.max_x());
  geometry.ny(mappings.max_y());
  geometry.nz(mappings.max_z());
  geometry.np(1);
}

std::string Config::debug() const {
  std::stringstream ss;
  if (builder)
    ss << builder->debug();
  else
    ss << "  ========           No Builder :(           ========\n";

  ss << "  Event position using weighted average: "
     << (analyzer.weighted() ? "YES" : "no") << "\n";

  ss << "  geometry_x = " << geometry.nx() << "\n";
  ss << "  geometry_y = " << geometry.ny() << "\n";
  ss << "  geometry_z = " << geometry.nz() << "\n";

  return ss.str();
}

}
