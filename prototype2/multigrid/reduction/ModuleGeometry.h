/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once

#include <cinttypes>
#include <string>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop

namespace Multigrid {

class ModuleGeometry {
public:
  uint32_t x_offset{0};
  uint32_t y_offset{0};
  uint32_t z_offset{0};

  void num_grids(uint16_t g);
  void num_wires(uint16_t w);
  void z_range(uint32_t z);
  void flipped_x(bool f);
  void flipped_z(bool f);
  bool flipped_x() const;
  bool flipped_z() const;

  uint16_t num_wires() const;
  uint16_t num_grids() const;

  uint32_t x_range() const;
  uint32_t y_range() const;
  uint32_t z_range() const;

  uint32_t x_from_wire(uint16_t w) const;
  uint32_t y_from_grid(uint16_t g) const;
  uint32_t z_from_wire(uint16_t w) const;

  std::string debug(std::string prefix = {}) const;

private:
  uint16_t grids_{40};
  uint16_t wires_{80};
  uint32_t z_range_{20};

  /// cached value, derived from z_range
  uint32_t x_range_{4};

  bool flipped_x_{false};
  bool flipped_z_{false};
};

void from_json(const nlohmann::json &j, ModuleGeometry &g);

}

