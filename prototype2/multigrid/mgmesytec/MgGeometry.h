/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once
#include <cinttypes>

class MgGeometry {

public:
  /** @brief identifies which channels are wires, from drawing by Anton */
  virtual bool isWire(uint16_t channel) = 0;

  /** @brief identifies which channels are grids, from drawing by Anton */
  virtual bool isGrid(uint16_t channel) = 0;

  /** @brief return the x coordinate of the detector */
  virtual uint32_t x(uint8_t bus, uint16_t channel) = 0;

  /** @brief return the y coordinate of the detector */
  virtual uint32_t y(uint8_t bus, uint16_t channel) = 0;

  /** @brief return the z coordinate of the detector */
  virtual uint32_t z(uint8_t bus, uint16_t channel) = 0;

protected:

  inline void swap(uint16_t &channel) {
    if (channel % 2 == 0) {
      channel += 1;
    } else {
      channel -= 1;
    }
  }

};
