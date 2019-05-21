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

class ModuleGeometry {
protected:
  std::vector<Filter> wire_filters_;
  std::vector<Filter> grid_filters_;

public:
  virtual ~ModuleGeometry() = default;

  uint16_t rescale_wire(uint16_t wire, uint16_t adc) const;

  uint16_t rescale_grid(uint16_t grid, uint16_t adc) const;

  bool valid_wire(uint16_t wire, uint16_t adc) const;

  bool valid_grid(uint16_t grid, uint16_t adc) const;

  void set_wire_filters(Filter mgf);

  void set_grid_filters(Filter mgf);

  void override_wire_filter(uint16_t n, Filter mgf);

  void override_grid_filter(uint16_t n, Filter mgf);

  virtual uint16_t max_channel() const = 0;

  virtual uint16_t max_wire() const = 0;

  virtual uint16_t max_grid() const = 0;

  virtual uint32_t max_x() const = 0;

  virtual uint32_t max_y() const = 0;

  virtual uint16_t max_z() const = 0;

  /** @brief identifies which channels are wires, from drawing by Anton */
  virtual bool isWire(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief identifies which channels are grids, from drawing by Anton */
  virtual bool isGrid(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief returns wire */
  virtual uint16_t wire(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief returns grid */
  virtual uint16_t grid(uint8_t VMM, uint16_t channel) const = 0;

  virtual uint32_t x_from_wire(uint16_t w) const = 0;

  /** @brief return the x coordinate of the detector */
  virtual uint32_t x(uint8_t VMM, uint16_t channel) const = 0;

  virtual uint32_t y_from_grid(uint16_t g) const = 0;

  /** @brief return the y coordinate of the detector */
  virtual uint32_t y(uint8_t VMM, uint16_t channel) const = 0;

  virtual uint32_t z_from_wire(uint16_t w) const = 0;

  /** @brief return the z coordinate of the detector */
  virtual uint32_t z(uint8_t VMM, uint16_t channel) const = 0;

  virtual std::string debug(std::string prefix = "") const;
};

void from_json(const nlohmann::json &j, ModuleGeometry &g);

}

