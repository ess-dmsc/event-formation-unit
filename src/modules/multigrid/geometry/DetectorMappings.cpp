/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates
 *
 * This currently (18/8 2018) is compatible with MG.Sequoia
 * detector demonstrators although not all channels may be in use
 */

#include <multigrid/geometry/DetectorMappings.h>

#include <multigrid/geometry/MGSeqMappings.h>
#include <multigrid/geometry/MG24Mappings.h>

#include <multigrid/geometry/PlaneMappings.h>

#include <common/Trace.h>
#include <fmt/format.h>
#include <sstream>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

std::string ModuleMapping::debug(std::string prefix) const {
  return fmt::format("plan_offset={}  grids[{}-{}]  wires[{}-{}]",
                     plane_offset,
                     grid_offset, (grid_offset + channel_mappings->max_grid()),
                     wire_offset, (wire_offset + channel_mappings->max_wire()))
      + channel_mappings->debug(prefix + "  ");
}

void DetectorMappings::add_bus(std::shared_ptr<ChannelMappings> mappings) {
  ModuleMapping modmap;
  modmap.channel_mappings = mappings;
  for (const auto &d : buses) {
    modmap.wire_offset += d.channel_mappings->max_wire();
    modmap.grid_offset += d.channel_mappings->max_grid();
  }
  modmap.plane_offset = 2 * buses.size();
  buses.push_back(modmap);
}

bool DetectorMappings::map(Hit &hit, uint8_t bus, uint16_t channel, uint16_t adc) const {
  if (bus >= buses.size()) {
    hit.plane = Hit::InvalidPlane;
    hit.coordinate = Hit::InvalidCoord;
    return false;
  }
  const auto &b = buses[bus];
  bool ret = b.channel_mappings->map(hit, channel, adc);

  // \todo this may not be necessary if module mappings are direct
  if (hit.plane != Hit::InvalidPlane)
    hit.plane += b.plane_offset;

  return ret;
}

Hit DetectorMappings::absolutify(const Hit &original) const {
  if ((original.plane == Hit::PulsePlane) || (original.plane == Hit::InvalidPlane))
    return original;

  Hit transformed = original;
  transformed.plane = plane_in_module(original.plane);
  const auto &b = buses[module_from_plane(original.plane)];
  if (transformed.plane == wire_plane)
    transformed.coordinate += b.wire_offset;
  else if (transformed.plane == grid_plane)
    transformed.coordinate += b.grid_offset;

  return transformed;
}

uint16_t DetectorMappings::max_wire() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.wire_offset + b.channel_mappings->max_wire();
}

uint16_t DetectorMappings::max_grid() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.grid_offset + b.channel_mappings->max_grid();
}

std::string DetectorMappings::debug(std::string prefix) const {
  std::stringstream ss;

  for (size_t i = 0; i < buses.size(); i++) {
    ss << prefix << "  bus#" << i << "   " << buses[i].debug(prefix + "  ");
  }

  return ss.str();
}

void from_json(const nlohmann::json &j, DetectorMappings &g) {
  for (unsigned int i = 0; i < j.size(); i++) {
    nlohmann::json jj = j[i];
    std::string type = jj["type"];
    if (type == "MGSeq") {
      auto geom = std::make_shared<MGSeqMappings>();
      (*geom) = jj;
      g.add_bus(geom);
    } else if (type == "MG24A") {
      auto geom = std::make_shared<MG24MappingsA>();
      (*geom) = jj;
      g.add_bus(geom);
    } else if (type == "MG24B") {
      auto geom = std::make_shared<MG24MappingsB>();
      (*geom) = jj;
      g.add_bus(geom);
    }
  }
}

}
