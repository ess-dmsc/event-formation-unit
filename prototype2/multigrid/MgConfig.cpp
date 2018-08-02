/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <multigrid/MgConfig.h>

#include <dataformats/multigrid/inc/json.h>
#include <fstream>
#include <sstream>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

MgConfig::MgConfig(std::string jsonfile) {
  Json::Value root{};
  Json::Reader reader{};
  std::ifstream t(jsonfile);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  if (!reader.parse(str, root, 0)) {
    XTRACE(INIT, WAR, "Invalid Json file: %s\n", jsonfile.c_str());
    return;
  }

  spoof_high_time = root["spoof_high_time"].asBool();

  swap_wires = root["swap_wires"].asBool();
  module = root["flipped_module"].asUInt();

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
  ss << "  Swap odd/even wires = " << (swap_wires ? "YES" : "no") << "\n";
  ss << "  Flip z coordinates on bus# = " << module << "\n";
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
