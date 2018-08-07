/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates
 *
 * This currently (18/8 2018) is compatible with MG.Sequoia
 * detector demonstrators although not all channels may be in use
 */

#pragma once

#include <multigrid/mgmesytec/BusGeometry.h>
#include <vector>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

class SequoiaGeometry {
private:
  struct BusDefinitionStruct {
    uint16_t wire_offset{0};
    uint16_t grid_offset{0};
    uint32_t x_offset{0};
    uint32_t y_offset{0};
    uint32_t z_offset{0};

    BusGeometry geometry;
  };

  std::vector<BusDefinitionStruct> buses;

public:

  inline void add_bus(BusGeometry geom) {
    BusDefinitionStruct g;
    g.geometry = geom;
    for (const auto &d : buses) {
      g.wire_offset += d.geometry.max_wire();
      g.grid_offset += d.geometry.max_grid();
      g.x_offset += d.geometry.max_x();
      //g.y_offset += d.y_offset + d.geometry.max_y();
      //g.z_offset += d.z_offset + d.geometry.max_z();
    }

    buses.push_back(g);
  }

  inline uint16_t rescale(uint8_t bus, uint16_t channel, uint16_t adc) const {
    if (bus >= buses.size())
      return adc;
    if (isWire(bus, channel)) {
      return buses.at(bus).geometry.rescale_wire(channel, adc);
    } else if (isGrid(bus, channel)) {
      return buses.at(bus).geometry.rescale_grid(channel, adc);
    }
    return adc;
  }

  inline bool is_valid(uint8_t bus, uint16_t channel, uint16_t adc) const {
    if (bus >= buses.size())
      return false;
    if (isWire(bus, channel)) {
      return buses.at(bus).geometry.valid_wire(channel, adc);
    } else if (isGrid(bus, channel)) {
      return buses.at(bus).geometry.valid_grid(channel, adc);
    }
    return false;
  }

  /** @brief identifies which channels are wires, from drawing by Anton */
  inline bool isWire(uint8_t bus, uint16_t channel) const {
    if (bus >= buses.size())
      return false;
    return buses.at(bus).geometry.isWire(channel);
  }

  /** @brief identifies which channels are grids, from drawing by Anton */
  inline bool isGrid(uint8_t bus, uint16_t channel) const {
    if (bus >= buses.size())
      return false;
    return buses.at(bus).geometry.isGrid(channel);
  }

  inline uint16_t wire(uint8_t bus, uint16_t channel) const {
    const auto &b = buses.at(bus);
    return b.wire_offset + b.geometry.wire(channel);
  }

  inline uint16_t grid(uint8_t bus, uint16_t channel) const {
    const auto &b = buses.at(bus);
    return b.grid_offset + b.geometry.grid(channel);
  }

  inline uint16_t max_wire() const {
    if (buses.empty())
      return 0;
    const auto &b = buses.at(buses.size() - 1);
    return b.wire_offset + b.geometry.max_wire();
  }

  inline uint16_t max_grid() const {
    if (buses.empty())
      return 0;
    const auto &b = buses.at(buses.size() - 1);
    return b.grid_offset + b.geometry.max_grid();
  }

  /** @brief return the x coordinate of the detector */
  inline uint32_t x(uint8_t bus, uint16_t channel) const {
    const auto &b = buses.at(bus);
    return b.x_offset + b.geometry.x(channel);
  }

  /** @brief return the y coordinate of the detector */
  inline uint32_t y(uint8_t bus, uint16_t channel) const {
    const auto &b = buses.at(bus);
    return b.y_offset + b.geometry.y(channel);
  }

  /** @brief return the z coordinate of the detector */
  inline uint32_t z(uint8_t bus, uint16_t channel) const {
    const auto &b = buses.at(bus);
    return b.z_offset + b.geometry.z(channel);
  }

  /** @brief return the x coordinate of the detector */
  inline uint32_t max_x() const {
    if (buses.empty())
      return 0;
    const auto &b = buses.at(buses.size() - 1);
    return b.x_offset + b.geometry.max_x();
  }

  /** @brief return the y coordinate of the detector */
  inline uint32_t max_y() const {
    if (buses.empty())
      return 0;
    const auto &b = buses.at(buses.size() - 1);
    return b.y_offset + b.geometry.max_y();
  }

  /** @brief return the z coordinate of the detector */
  inline uint32_t max_z() const {
    if (buses.empty())
      return 0;
    const auto &b = buses.at(buses.size() - 1);
    return b.z_offset + b.geometry.max_z();
  }

  std::string debug(std::string prefix) const {
    std::stringstream ss;

    for (size_t i = 0; i < buses.size(); i++) {
      ss << prefix << "  Bus#" << i << "   "
         << "grid+" << buses.at(i).grid_offset << "  "
         << "wire+" << buses.at(i).wire_offset << "  "
         << "x+" << buses.at(i).x_offset << "  "
         << "y+" << buses.at(i).y_offset << "  "
         << "z+" << buses.at(i).z_offset << "  "
         << "\n";
      ss << buses.at(i).geometry.debug(prefix + "    ");
    }

    return ss.str();
  }

};

}
