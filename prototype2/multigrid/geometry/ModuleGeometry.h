/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once
#include <multigrid/geometry/Filter.h>

#include <cinttypes>
#include <string>
#include <vector>

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

  virtual uint32_t x_from_wire(uint16_t w) const;
  virtual uint32_t y_from_grid(uint16_t g) const;
  virtual uint32_t z_from_wire(uint16_t w) const;

  // \todo default arg
  virtual std::string debug(std::string prefix) const;

private:
  uint16_t max_grid_{40};
  uint16_t max_wire_{80};
  uint16_t max_z_{20};

  bool flipped_x_{false};
  bool flipped_z_{false};
};

void from_json(const nlohmann::json &j, ModuleLogicalGeometry &g);


class ModuleChannelMappings {
public:
  FilterSet wire_filters;
  FilterSet grid_filters;

  virtual ~ModuleChannelMappings() = default;

  virtual uint16_t max_channel() const = 0;

  /** @brief identifies which channels are wires, from drawing by Anton */
  virtual bool isWire(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief identifies which channels are grids, from drawing by Anton */
  virtual bool isGrid(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief returns wire */
  virtual uint16_t wire(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief returns grid */
  virtual uint16_t grid(uint8_t VMM, uint16_t channel) const = 0;

  virtual std::string debug(std::string prefix) const;
};

void from_json(const nlohmann::json &j, ModuleChannelMappings &g);

}

