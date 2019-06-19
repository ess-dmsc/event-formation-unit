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

uint16_t DigitalGeometry::rescale(uint8_t bus, uint16_t channel, uint16_t adc) const {
  if (bus >= buses.size())
    return adc;
  const auto &b = buses[bus].channel_mappings;
  if (isWire(bus, channel)) {
    return b->wire_filters.rescale(b->wire(channel), adc);
  } else if (isGrid(bus, channel)) {
    return b->grid_filters.rescale(b->grid(channel), adc);
  }
  return adc;
}

bool DigitalGeometry::is_valid(uint8_t bus, uint16_t channel, uint16_t adc) const {
  if (bus >= buses.size())
    return false;
  const auto &b = buses[bus].channel_mappings;
  if (isWire(bus, channel)) {
    return b->wire_filters.valid(b->wire(channel), adc);
  } else if (isGrid(bus, channel)) {
    return b->grid_filters.valid(b->grid(channel), adc);
  }
  return false;
}

/** @brief identifies which channels are wires, from drawing by Anton */
bool DigitalGeometry::isWire(uint8_t bus, uint16_t channel) const {
  if (bus >= buses.size())
    return false;
  return buses[bus].channel_mappings->isWire(channel);
}

/** @brief identifies which channels are grids, from drawing by Anton */
bool DigitalGeometry::isGrid(uint8_t bus, uint16_t channel) const {
  if (bus >= buses.size())
    return false;
  return buses[bus].channel_mappings->isGrid(channel);
}

uint16_t DigitalGeometry::wire(uint8_t bus, uint16_t channel) const {
  const auto &b = buses[bus];
  return b.wire_offset + b.channel_mappings->wire(channel);
}

uint16_t DigitalGeometry::grid(uint8_t bus, uint16_t channel) const {
  const auto &b = buses[bus];
  return b.grid_offset + b.channel_mappings->grid(channel);
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
