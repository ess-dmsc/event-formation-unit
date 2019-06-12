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

class ModuleGeometry : public ModuleLogicalGeometry {
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

  /** @brief identifies which channels are wires, from drawing by Anton */
  virtual bool isWire(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief identifies which channels are grids, from drawing by Anton */
  virtual bool isGrid(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief returns wire */
  virtual uint16_t wire(uint8_t VMM, uint16_t channel) const = 0;

  /** @brief returns grid */
  virtual uint16_t grid(uint8_t VMM, uint16_t channel) const = 0;

  std::string debug(std::string prefix) const override;
};

void from_json(const nlohmann::json &j, ModuleGeometry &g);

}

