/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once
#include <multigrid/geometry/ModuleGeometry.h>

#include <cinttypes>
#include <string>
#include <vector>

namespace Multigrid {

class MGSeqGeometry : public ModuleGeometry {
public:

  // Configuration
  void swap_wires(bool s);
  void swap_grids(bool s);
  void max_wire(uint16_t g);
  void max_channel(uint16_t g);
  void max_z(uint16_t w);
  void flipped_x(bool f);
  void flipped_z(bool f);
  bool swap_wires() const;
  bool swap_grids() const;
  bool flipped_x() const;
  bool flipped_z() const;

  // Implementation

  uint16_t max_channel() const override;
  uint16_t max_wire() const override;
  uint16_t max_grid() const override;

  uint32_t max_x() const override;
  uint32_t max_y() const override;
  uint16_t max_z() const override;

  /** @brief identifies which channels are wires, from drawing by Anton */
  bool isWire(uint8_t VMM, uint16_t channel) const;

  /** @brief identifies which channels are grids, from drawing by Anton */
  bool isGrid(uint8_t VMM, uint16_t channel) const;

  /** @brief returns wire */
  uint16_t wire(uint8_t VMM, uint16_t channel) const;

  /** @brief returns grid */
  uint16_t grid(uint8_t VMM, uint16_t channel) const;

  uint32_t x_from_wire(uint16_t w) const;

  /** @brief return the x coordinate of the detector */
  uint32_t x(uint8_t VMM, uint16_t channel) const;

  uint32_t y_from_grid(uint16_t g) const;

  /** @brief return the y coordinate of the detector */
  uint32_t y(uint8_t VMM, uint16_t channel) const;

  uint32_t z_from_wire(uint16_t w) const;

  /** @brief return the z coordinate of the detector */
  uint32_t z(uint8_t VMM, uint16_t channel) const;

  std::string debug(std::string prefix = "") const override;

protected:
  static void swap(uint16_t &channel);

  uint16_t max_channel_{120};
  uint16_t max_wire_{80};
  uint16_t max_z_{20};

  bool flipped_x_{false};
  bool flipped_z_{false};

  bool swap_wires_{false};
  bool swap_grids_{false};
};

void from_json(const nlohmann::json &j, MGSeqGeometry &g);

}

