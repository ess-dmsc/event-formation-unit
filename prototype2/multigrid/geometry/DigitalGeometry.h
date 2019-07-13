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

namespace Multigrid {

struct ModuleGeometry {
  uint32_t x_offset{0};
  uint32_t y_offset{0};
  uint32_t z_offset{0};

  ModuleLogicalGeometry logical_geometry;

  std::string debug(std::string prefix = "") const;
};

void from_json(const nlohmann::json &j, ModuleGeometry &g);

class DetectorGeometry {
public:

  void add_bus(ModuleLogicalGeometry geom);

  /** @brief return the x coordinate of the detector */
  uint32_t x_from_wire(size_t module, uint16_t w) const;

  /** @brief return the y coordinate of the detector */
  uint32_t y_from_grid(size_t module, uint16_t g) const;

  /** @brief return the z coordinate of the detector */
  uint32_t z_from_wire(size_t module, uint16_t w) const;

  /** @brief return the x coordinate of the detector */
  uint32_t max_x() const;

  /** @brief return the y coordinate of the detector */
  uint32_t max_y() const;

  /** @brief return the z coordinate of the detector */
  uint32_t max_z() const;

  std::string debug(std::string prefix = "") const;

//private:
  std::vector<ModuleGeometry> buses;
};

void from_json(const nlohmann::json &j, DetectorGeometry &g);

}