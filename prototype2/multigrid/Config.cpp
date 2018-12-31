/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/Config.h>

#include <fstream>
#include <sstream>
#include <multigrid/generators/BuilderReadouts.h>
#include <multigrid/mesytec/BuilderMesytec.h>

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
    LOG(INIT, Sev::Warning, "Invalid Json file: {}", jsonfile);
    return;
  }

  SequoiaGeometry mappings;
  auto m = root["mappings"];
  for (unsigned int i = 0; i < m.size(); i++) {
    mappings.add_bus(m[i]);
  }

  auto br = root["builder"];
  if (br["type"] == "mesytec") {
    auto mbuilder = std::make_shared<BuilderMesytec>();
    mbuilder->digital_geometry = mappings;
    mbuilder->vmmr16Parser.spoof_high_time(br["spoof_high_time"]);
    builder = mbuilder;
  } else if (br["type"] == "readouts") {
    auto rbuilder = std::make_shared<BuilderReadouts>();
    rbuilder->digital_geometry = mappings;
    builder = rbuilder;
  }

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
  ss << "  ===============================================\n";
  ss << "  ========           MultiGrid           ========\n";
  ss << "  ===============================================\n";

//  ss << "  Spoof high time = " << (spoof_high_time ? "YES" : "no") << "\n";
//
  ss << "  Geometry mappings:\n";
  ss << analyzer.mappings.debug("  ") << "\n";

  ss << "  Event position using weighted average: "
     << (analyzer.weighted() ? "YES" : "no") << "\n";

  ss << "  geometry_x = " << geometry.nx() << "\n";
  ss << "  geometry_y = " << geometry.ny() << "\n";
  ss << "  geometry_z = " << geometry.nz() << "\n";

  return ss.str();
}

}