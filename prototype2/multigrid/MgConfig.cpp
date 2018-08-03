/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/MgConfig.h>

#include <dataformats/multigrid/inc/json.h>
#include <fstream>
#include <sstream>

#include <common/Log.h>

MgConfig::MgConfig(std::string jsonfile) {
  Json::Value root{};
  Json::Reader reader{};
  std::ifstream t(jsonfile);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  if (!reader.parse(str, root, 0)) {
    LOG(Sev::Warning, "Invalid Json file: {}", jsonfile);
    return;
  }

  spoof_high_time = root["spoof_high_time"].asBool();

  auto m = root["geometry_mappings"];
  for (unsigned int index = 0; index < m.size(); index++) {
    MgBusGeometry g;
    g.swap_wires(m[index]["swap_wires"].asBool());
    g.swap_grids(m[index]["swap_grids"].asBool());
    g.flipped_x(m[index]["flipped_x"].asBool());
    g.flipped_z(m[index]["flipped_z"].asBool());
    mappings.add_bus(g);
  }

  reduction_strategy = root["reduction_strategy"].asString();

//  wireThresholdLo = root["wireThresholdLo"].asUInt();
//  wireThresholdHi = root["wireThresholdHi"].asUInt();
//  gridThresholdLo = root["gridThresholdLo"].asUInt();
//  gridThresholdHi = root["gridThresholdHi"].asUInt();

  // \todo deduce geometry from MG mappings?
  geometry.nx(root["geometry_x"].asInt());
  geometry.ny(root["geometry_y"].asInt());
  geometry.nz(root["geometry_z"].asInt());
  geometry.np(1);
}

std::string MgConfig::debug() const {
  std::stringstream ss;
  ss << "  ===============================================\n";
  ss << "  ========       multigrid mesytec       ========\n";
  ss << "  ===============================================\n";

  ss << "  Spoof high time = " << (spoof_high_time ? "YES" : "no") << "\n";

  ss << "  Geometry mappings:\n";
  ss << mappings.debug("  ") << "\n";

  ss << "  Event reduction strategy: " << reduction_strategy << "\n";
  if (reduction_strategy == "maximum") {
    ss << "    Thresholds:\n";
    ss << "     wire min = " << wireThresholdLo << "\n";
    ss << "     wire max = " << wireThresholdHi << "\n";
    ss << "     grid min = " << gridThresholdLo << "\n";
    ss << "     grid max = " << gridThresholdHi << "\n";
  }
  ss << "  geometry_x = " << geometry.nx() << "\n";
  ss << "  geometry_y = " << geometry.ny() << "\n";
  ss << "  geometry_z = " << geometry.nz() << "\n";

  return ss.str();
}
