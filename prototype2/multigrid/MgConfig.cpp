/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/MgConfig.h>

#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

#include <common/Log.h>

namespace Multigrid {

Config::Config(std::string jsonfile) {
  nlohmann::json root;
  try {
    std::ifstream t(jsonfile);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    root = nlohmann::json::parse(str);
  }
  catch (...) {
    LOG(Sev::Warning, "Invalid Json file: {}", jsonfile);
    return;
  }

  spoof_high_time = root["spoof_high_time"];

  auto m = root["geometry_mappings"];
  for (unsigned int i = 0; i < m.size(); i++) {
    mappings.add_bus(m[i]);
  }

  reduction_strategy = root["reduction_strategy"];

  // deduced geometry from MG mappings
  geometry.nx(mappings.max_x());
  geometry.ny(mappings.max_y());
  geometry.nz(mappings.max_z());
  geometry.np(1);
}

std::string Config::debug() const {
  std::stringstream ss;
  ss << "  ===============================================\n";
  ss << "  ========       multigrid mesytec       ========\n";
  ss << "  ===============================================\n";

  ss << "  Spoof high time = " << (spoof_high_time ? "YES" : "no") << "\n";

  ss << "  Geometry mappings:\n";
  ss << mappings.debug("  ") << "\n";

  ss << "  Event reduction strategy: " << reduction_strategy << "\n";

  ss << "  geometry_x = " << geometry.nx() << "\n";
  ss << "  geometry_y = " << geometry.ny() << "\n";
  ss << "  geometry_z = " << geometry.nz() << "\n";

  return ss.str();
}

}