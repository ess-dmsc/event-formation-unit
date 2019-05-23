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

#include <multigrid/geometry/MGSeqGeometry.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

void DigitalGeometry::add_bus(std::shared_ptr<ModuleGeometry> geom) {
  BusDefinitionStruct g;
  g.geometry = geom;
  for (const auto &d : buses) {
    g.wire_offset += d.geometry->max_wire();
    g.grid_offset += d.geometry->max_grid();
    g.x_offset += d.geometry->max_x();
    //g.y_offset += d.y_offset + d.geometry.max_y();
    //g.z_offset += d.z_offset + d.geometry.max_z();
  }

  buses.push_back(g);
}

uint16_t DigitalGeometry::rescale(uint8_t FEC, uint8_t VMM, uint16_t channel, uint16_t adc) const {
  if (FEC >= buses.size())
    return adc;
  if (isWire(FEC, VMM, channel)) {
    const auto &b = buses[FEC].geometry;
    return b->rescale_wire(b->wire(VMM, channel), adc);
  } else if (isGrid(FEC, VMM, channel)) {
    const auto &b = buses[FEC].geometry;
    return b->rescale_grid(b->grid(VMM, channel), adc);
  }
  return adc;
}

bool DigitalGeometry::is_valid(uint8_t FEC, uint8_t VMM, uint16_t channel, uint16_t adc) const {
  if (FEC >= buses.size())
    return false;
  if (isWire(FEC, VMM, channel)) {
    const auto &b = buses[FEC].geometry;
    return b->valid_wire(b->wire(VMM, channel), adc);
  } else if (isGrid(FEC, VMM, channel)) {
    const auto &b = buses[FEC].geometry;
    return b->valid_grid(b->grid(VMM, channel), adc);
  }
  return false;
}

/** @brief identifies which channels are wires, from drawing by Anton */
bool DigitalGeometry::isWire(uint8_t FEC, uint8_t VMM, uint16_t channel) const {
  if (FEC >= buses.size())
    return false;
  return buses[FEC].geometry->isWire(VMM, channel);
}

/** @brief identifies which channels are grids, from drawing by Anton */
bool DigitalGeometry::isGrid(uint8_t FEC, uint8_t VMM, uint16_t channel) const {
  if (FEC >= buses.size())
    return false;
  return buses[FEC].geometry->isGrid(VMM, channel);
}

uint16_t DigitalGeometry::wire(uint8_t FEC, uint8_t VMM, uint16_t channel) const {
  const auto &b = buses[FEC];
  return b.wire_offset + b.geometry->wire(VMM, channel);
}

uint16_t DigitalGeometry::grid(uint8_t FEC, uint8_t VMM, uint16_t channel) const {
  const auto &b = buses[FEC];
  return b.grid_offset + b.geometry->grid(VMM, channel);
}

uint16_t DigitalGeometry::max_wire() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.wire_offset + b.geometry->max_wire();
}

uint16_t DigitalGeometry::max_grid() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.grid_offset + b.geometry->max_grid();
}

uint32_t DigitalGeometry::x_from_wire(uint16_t w) const {
  auto b = buses.begin();
  while (w >= (b->wire_offset + b->geometry->max_wire()))
    ++b;
  return b->x_offset + b->geometry->x_from_wire(w - b->wire_offset);
}

/** @brief return the y coordinate of the detector */
uint32_t DigitalGeometry::y_from_grid(uint16_t g) const {
  auto b = buses.begin();
  while (g >= (b->grid_offset + b->geometry->max_grid())) {
    ++b;
  }
  return b->y_offset + b->geometry->y_from_grid(g - b->grid_offset);
}

/** @brief return the z coordinate of the detector */
uint32_t DigitalGeometry::z_from_wire(uint16_t w) const {
  auto b = buses.begin();
  while (w >= (b->wire_offset + b->geometry->max_wire()))
    ++b;
  return b->z_offset + b->geometry->z_from_wire(w - b->wire_offset);
}

/** @brief return the x coordinate of the detector */
uint32_t DigitalGeometry::max_x() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.x_offset + b.geometry->max_x();
}

/** @brief return the y coordinate of the detector */
uint32_t DigitalGeometry::max_y() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.y_offset + b.geometry->max_y();
}

/** @brief return the z coordinate of the detector */
uint32_t DigitalGeometry::max_z() const {
  if (buses.empty())
    return 0;
  const auto &b = buses.back();
  return b.z_offset + b.geometry->max_z();
}

std::string DigitalGeometry::debug(std::string prefix) const {
  std::stringstream ss;

  for (size_t i = 0; i < buses.size(); i++) {
    ss << prefix << "  FEC#" << i << "   "
       << "grid+" << buses[i].grid_offset << "  "
       << "wire+" << buses[i].wire_offset << "  "
       << "x+" << buses[i].x_offset << "  "
       << "y+" << buses[i].y_offset << "  "
       << "z+" << buses[i].z_offset << "  "
       << "\n";
    ss << buses[i].geometry->debug(prefix + "    ");
  }

  return ss.str();
}

void from_json(const nlohmann::json &j, DigitalGeometry &g) {
  for (unsigned int i = 0; i < j.size(); i++) {
    auto geom = std::make_shared<MGSeqGeometry>();
    (*geom) = j[i];
    g.add_bus(geom);
  }
}

}
