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

#include <multigrid/geometry/ModuleGeometry.h>
#include <multigrid/geometry/ChannelMappings.h>

namespace Multigrid {

struct BusDefinitionStruct {
  uint16_t wire_offset{0};
  uint16_t grid_offset{0};
  uint32_t x_offset{0};
  uint32_t y_offset{0};
  uint32_t z_offset{0};

  ModuleLogicalGeometry logical_geometry;

  std::shared_ptr<ChannelMappings> channel_mappings;

  std::string debug(std::string prefix = "") const;
};

void from_json(const nlohmann::json &j, BusDefinitionStruct &g);

class DigitalGeometry {
private:
  std::vector<BusDefinitionStruct> buses;

public:

  void add_bus(BusDefinitionStruct geom);

  // \todo document this
  bool map(Hit& hit, uint8_t bus, uint16_t channel, uint16_t adhc) const;

  // \todo document this
  // plane becomes 0 or 1, regardless of module
  // coord becomes total grid/wire number
  Hit absolutify(const Hit& original) const;

  uint16_t max_wire() const;

  uint16_t max_grid() const;

  uint32_t x_from_wire(uint16_t w) const;

  /** @brief return the y coordinate of the detector */
  uint32_t y_from_grid(uint16_t g) const;

  /** @brief return the z coordinate of the detector */
  uint32_t z_from_wire(uint16_t w) const;

  /** @brief return the x coordinate of the detector */
  uint32_t max_x() const;

  /** @brief return the y coordinate of the detector */
  uint32_t max_y() const;

  /** @brief return the z coordinate of the detector */
  uint32_t max_z() const;

  std::string debug(std::string prefix = "") const;
};

void from_json(const nlohmann::json &j, DigitalGeometry &g);

}
