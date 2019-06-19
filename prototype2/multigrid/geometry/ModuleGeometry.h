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
#include <nlohmann/json.hpp>

namespace Multigrid {

class ModuleLogicalGeometry {
public:
  void max_grid(uint16_t g);
  void max_wire(uint16_t w);
  void max_z(uint16_t w);
  void flipped_x(bool f);
  void flipped_z(bool f);
  bool flipped_x() const;
  bool flipped_z() const;

  uint16_t max_wire() const;
  uint16_t max_grid() const;

  uint32_t max_x() const;
  uint32_t max_y() const;
  uint16_t max_z() const;

  uint32_t x_from_wire(uint16_t w) const;
  uint32_t y_from_grid(uint16_t g) const;
  uint32_t z_from_wire(uint16_t w) const;

  // \todo default arg
  std::string debug(std::string prefix) const;

private:
  uint16_t max_grid_{40};
  uint16_t max_wire_{80};
  uint16_t max_z_{20};

  bool flipped_x_{false};
  bool flipped_z_{false};
};

void from_json(const nlohmann::json &j, ModuleLogicalGeometry &g);

}

