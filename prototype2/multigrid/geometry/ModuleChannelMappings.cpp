/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#include <multigrid/geometry/ModuleChannelMappings.h>

#include <fmt/format.h>
#include <sstream>

namespace Multigrid {

std::string ModuleChannelMappings::debug(std::string prefix) const {
  std::stringstream ss;

  auto wf = wire_filters.debug(prefix + "  ");
  if (!wf.empty()) {
    ss << prefix << "Wire filters:\n" << wf;
  }

  auto gf = grid_filters.debug(prefix + "  ");
  if (!gf.empty()) {
    ss << prefix << "Grid filters:\n" << gf;
  }

  return ss.str();
}

void from_json(const nlohmann::json &j, ModuleChannelMappings &g) {
  if (j.count("wire_filters")) {
    g.wire_filters = j["wire_filters"];
  }

  if (j.count("grid_filters")) {
    g.grid_filters = j["grid_filters"];
  }

}

}

