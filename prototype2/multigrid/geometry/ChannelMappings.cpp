/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#include <multigrid/geometry/ChannelMappings.h>
#include <multigrid/geometry/PlaneMappings.h>

#include <sstream>

namespace Multigrid {

bool ChannelMappings::map(Hit &hit, uint16_t channel, uint16_t adc) const {
  if (this->isWire(channel)) {
    hit.coordinate = this->wire(channel);
    hit.plane = wire_plane;
    hit.weight = wire_filters.rescale(hit.coordinate, adc);
    if (!wire_filters.valid(hit.coordinate, hit.weight)) {
      return false;
    }
  } else if (this->isGrid(channel)) {
    hit.coordinate = this->grid(channel);
    hit.plane = grid_plane;
    hit.weight = grid_filters.rescale(hit.coordinate, adc);
    if (!grid_filters.valid(hit.coordinate, hit.weight)) {
      return false;
    }
  } else {
    hit.plane = Hit::InvalidPlane;
    hit.coordinate = Hit::InvalidCoord;
    return false;
  }
  return true;
}

std::string ChannelMappings::debug(std::string prefix) const {
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

void from_json(const nlohmann::json &j, ChannelMappings &g) {
  if (j.count("wire_filters")) {
    g.wire_filters = j["wire_filters"];
  }

  if (j.count("grid_filters")) {
    g.grid_filters = j["grid_filters"];
  }

}

}

