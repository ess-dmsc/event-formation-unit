/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/MgConfig.h>

#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

#include <common/Log.h>

MgFilter get_filter(nlohmann::json& v)
{
  MgFilter ret;
  ret.minimum = v["min"];
  ret.maximum = v["max"];
  ret.rescale = v["rescale"];
  return ret;
}

MgConfig::MgConfig(std::string jsonfile) {

  std::ifstream t(jsonfile);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());


  nlohmann::json root;

  try {
    root = nlohmann::json::parse(str);
  }
  catch (...)
  {
    LOG(Sev::Warning, "Invalid Json file: {}", jsonfile);
    return;
  }

  spoof_high_time = root["spoof_high_time"];

  auto m = root["geometry_mappings"];
  for (unsigned int i = 0; i < m.size(); i++) {
    auto mi = m[i];

    MgBusGeometry g;
    g.max_channel(mi["max_channel"]);
    g.max_wire(mi["max_wire"]);
    g.max_z(mi["max_z"]);

    g.swap_wires(mi["swap_wires"]);
    g.swap_grids(mi["swap_grids"]);
    g.flipped_x(mi["flipped_x"]);
    g.flipped_z(mi["flipped_z"]);

    auto wf = mi["wire_filters"];
    g.set_wire_filters(get_filter(wf["blanket"]));
    auto wfe = wf["exceptions"];
    for (unsigned int j = 0; j < wfe.size(); j++) {
      uint16_t idx = wfe[j]["idx"];
      g.override_wire_filter(idx, get_filter(wfe[j]));
    }

    auto gf = mi["grid_filters"];
    g.set_grid_filters(get_filter(gf["blanket"]));
    auto gfe = gf["exceptions"];
    for (unsigned int j = 0; j < gfe.size(); j++) {
      uint16_t idx = gfe[j]["idx"];
      g.override_grid_filter(idx, get_filter(gfe[j]));
    }

    mappings.add_bus(g);
  }

  reduction_strategy = root["reduction_strategy"];

  // deduced geometry from MG mappings
  geometry.nx(mappings.max_x());
  geometry.ny(mappings.max_y());
  geometry.nz(mappings.max_z());
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

  ss << "  geometry_x = " << geometry.nx() << "\n";
  ss << "  geometry_y = " << geometry.ny() << "\n";
  ss << "  geometry_z = " << geometry.nz() << "\n";

  return ss.str();
}
