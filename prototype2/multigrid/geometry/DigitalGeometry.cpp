/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates
 *
 * This currently (18/8 2018) is compatible with MG.Sequoia
 * detector demonstrators although not all channels may be in use
 */

#include <multigrid/geometry/DigitalGeometry.h>

#include <multigrid/geometry/MGSeqMappings.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

std::string ModuleMapping::debug(std::string prefix) const {
  return fmt::format("grids[{}-{}] wires[{}-{}]",
                     grid_offset, (grid_offset + grids), wire_offset, (wire_offset + wires))
      + channel_mappings->debug(prefix + "  ");
}

void from_json(const nlohmann::json &j, ModuleMapping &g) {
  // \todo type, bounds
  auto geom = std::make_shared<MGSeqMappings>();
  (*geom) = j;
  g.channel_mappings = geom;
}

void DetectorMapping::add_bus(ModuleMapping geom, uint16_t wires, uint16_t grids) {
  for (const auto &d : buses) {
    geom.wire_offset += d.wires;
    geom.grid_offset += d.grids;
  }
  geom.wires = wires;
  geom.grids = grids;
  buses.push_back(geom);
}

bool DetectorMapping::map(Hit &hit, uint8_t bus, uint16_t channel, uint16_t adc) const {
  if (bus >= buses.size()) {
    hit.plane = Hit::InvalidPlane;
    hit.coordinate = Hit::InvalidCoord;
    return false;
  }
  const auto &b = buses[bus];
  bool ret = b.channel_mappings->map(hit, channel, adc);

  // \todo this may not be necessary if module mappings are direct
  if (hit.plane != Hit::InvalidPlane)
    hit.plane += 2 * bus;

  return ret;
}

Hit DetectorMapping::absolutify(const Hit &original) const {
  Hit transformed = original;
  transformed.plane = original.plane % 2;
  const auto &b = buses[original.plane / 2];
  if (transformed.plane == ChannelMappings::wire_plane)
    transformed.coordinate += b.wire_offset;
  else if (transformed.plane == ChannelMappings::grid_plane)
    transformed.coordinate += b.grid_offset;

  // \todo return transformed
  return original;
}

uint16_t DetectorMapping::max_wire() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.wire_offset + b.wires;
}

uint16_t DetectorMapping::max_grid() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.grid_offset + b.grids;
}

std::string DetectorMapping::debug(std::string prefix) const {
  std::stringstream ss;

  for (size_t i = 0; i < buses.size(); i++) {
    ss << prefix << "  bus#" << i << "   " << buses[i].debug(prefix);
  }

  return ss.str();
}

void from_json(const nlohmann::json &j, DetectorMapping &g) {
  for (unsigned int i = 0; i < j.size(); i++) {
    // \todo this is wrong
    g.add_bus(j[i], 100, 100);
  }
}

std::string ModuleGeometry::debug(std::string prefix) const {
  std::stringstream ss;
  ss << "x+" << x_offset << "  "
     << "y+" << y_offset << "  "
     << "z+" << z_offset << "  "
     << "\n";
  ss << logical_geometry.debug(prefix + "  ");
  return ss.str();
}

void from_json(const nlohmann::json &j, ModuleGeometry &g) {
  // \todo set max grid and wire
  g.logical_geometry = j["geometry"];
  //.max_grid(geom->max_channel() - geom->max_wire());
}

void DetectorGeometry::add_bus(ModuleGeometry geom) {
  for (const auto &d : buses) {
    geom.x_offset += d.logical_geometry.max_x();
    //g.y_offset += d.y_offset + d.channel_mappings.max_y();
    //g.z_offset += d.z_offset + d.channel_mappings.max_z();
  }
  buses.push_back(geom);
}

uint32_t DetectorGeometry::x_from_wire(size_t module, uint16_t w) const {
  const auto &b = buses[module];
  return b.x_offset + b.logical_geometry.x_from_wire(w);
}

/** @brief return the y coordinate of the detector */
uint32_t DetectorGeometry::y_from_grid(size_t module, uint16_t g) const {
  const auto &b = buses[module];
  return b.y_offset + b.logical_geometry.y_from_grid(g);
}

/** @brief return the z coordinate of the detector */
uint32_t DetectorGeometry::z_from_wire(size_t module, uint16_t w) const {
  const auto &b = buses[module];
  return b.z_offset + b.logical_geometry.z_from_wire(w);
}

/** @brief return the x coordinate of the detector */
uint32_t DetectorGeometry::max_x() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.x_offset + b.logical_geometry.max_x();
}

/** @brief return the y coordinate of the detector */
uint32_t DetectorGeometry::max_y() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.y_offset + b.logical_geometry.max_y();
}

/** @brief return the z coordinate of the detector */
uint32_t DetectorGeometry::max_z() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.z_offset + b.logical_geometry.max_z();
}

std::string DetectorGeometry::debug(std::string prefix) const {
  std::stringstream ss;
  for (size_t i = 0; i < buses.size(); i++) {
    ss << prefix << "  bus#" << i << "   " << buses[i].debug(prefix);
  }
  return ss.str();
}

void from_json(const nlohmann::json &j, DetectorGeometry &g) {
  for (unsigned int i = 0; i < j.size(); i++) {
    g.add_bus(j[i]);
  }
}

std::string BusDefinitionStruct::debug(std::string prefix) const {
  std::stringstream ss;

  ss << "grid+" << grid_offset << "  "
     << "wire+" << wire_offset << "  "
     << "x+" << x_offset << "  "
     << "y+" << y_offset << "  "
     << "z+" << z_offset << "  "
     << "\n";
  ss << logical_geometry.debug(prefix + "  ");
  ss << channel_mappings->debug(prefix + "  ");

  return ss.str();
}

void from_json(const nlohmann::json &j, BusDefinitionStruct &g) {
  auto geom = std::make_shared<MGSeqMappings>();
  (*geom) = j;
  g.channel_mappings = geom;
  g.logical_geometry.max_grid(geom->max_channel() - geom->max_wire());
}

void DigitalGeometry::add_bus(BusDefinitionStruct geom) {
  for (const auto &d : buses) {
    geom.wire_offset += d.logical_geometry.max_wire();
    geom.grid_offset += d.logical_geometry.max_grid();
    geom.x_offset += d.logical_geometry.max_x();
    //g.y_offset += d.y_offset + d.channel_mappings.max_y();
    //g.z_offset += d.z_offset + d.channel_mappings.max_z();
  }
  buses.push_back(geom);
}

bool DigitalGeometry::map(Hit &hit, uint8_t bus, uint16_t channel, uint16_t adc) const {
  if (bus >= buses.size()) {
    hit.plane = Hit::InvalidPlane;
    hit.coordinate = Hit::InvalidCoord;
    return false;
  }
  const auto &b = buses[bus];
  bool ret = b.channel_mappings->map(hit, channel, adc);

  // \todo remove this, use absolutify
  if (hit.plane == ChannelMappings::wire_plane)
    hit.coordinate += b.wire_offset;
  else if (hit.plane == ChannelMappings::grid_plane)
    hit.coordinate += b.grid_offset;
  return ret;
}

Hit DigitalGeometry::absolutify(const Hit &original) const {
  Hit transformed = original;
  transformed.plane = original.plane % 2;
  const auto &b = buses[original.plane / 2];
  if (transformed.plane == ChannelMappings::wire_plane)
    transformed.coordinate += b.wire_offset;
  else if (transformed.plane == ChannelMappings::grid_plane)
    transformed.coordinate += b.grid_offset;

  // \todo return transformed
  return original;
}

uint16_t DigitalGeometry::max_wire() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.wire_offset + b.logical_geometry.max_wire();
}

uint16_t DigitalGeometry::max_grid() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.grid_offset + b.logical_geometry.max_grid();
}

uint32_t DigitalGeometry::x_from_wire(uint16_t w) const {
  auto b = buses.begin();
  while (w >= (b->wire_offset + b->logical_geometry.max_wire()))
    ++b;
  return b->x_offset + b->logical_geometry.x_from_wire(w - b->wire_offset);
}

/** @brief return the y coordinate of the detector */
uint32_t DigitalGeometry::y_from_grid(uint16_t g) const {
  auto b = buses.begin();
  while (g >= (b->grid_offset + b->logical_geometry.max_grid())) {
    ++b;
  }
  return b->y_offset + b->logical_geometry.y_from_grid(g - b->grid_offset);
}

/** @brief return the z coordinate of the detector */
uint32_t DigitalGeometry::z_from_wire(uint16_t w) const {
  auto b = buses.begin();
  while (w >= (b->wire_offset + b->logical_geometry.max_wire()))
    ++b;
  return b->z_offset + b->logical_geometry.z_from_wire(w - b->wire_offset);
}

/** @brief return the x coordinate of the detector */
uint32_t DigitalGeometry::max_x() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.x_offset + b.logical_geometry.max_x();
}

/** @brief return the y coordinate of the detector */
uint32_t DigitalGeometry::max_y() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.y_offset + b.logical_geometry.max_y();
}

/** @brief return the z coordinate of the detector */
uint32_t DigitalGeometry::max_z() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.z_offset + b.logical_geometry.max_z();
}

std::string DigitalGeometry::debug(std::string prefix) const {
  std::stringstream ss;

  for (size_t i = 0; i < buses.size(); i++) {
    ss << prefix << "  bus#" << i << "   " << buses[i].debug(prefix);
  }

  return ss.str();
}

void from_json(const nlohmann::json &j, DigitalGeometry &g) {
  for (unsigned int i = 0; i < j.size(); i++) {
    g.add_bus(j[i]);
  }
}

}
