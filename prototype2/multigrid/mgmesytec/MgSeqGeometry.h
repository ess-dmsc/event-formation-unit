/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizere, channels) and (x,y,z) coordinates
 *
 * This currently (18/8 2018) is compatible with MG.Saquoia
 * detector demonstrators although not all channels may be in use
 */

#pragma once

// \todo map to grids and wires before xyz, subtract to get grid numbers

#include <multigrid/mgmesytec/MgGeometry.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class MgSeqGeometry : public MgGeometry {
private:
  // \todo per bus
  bool swap_wires_on{false};
  // \todo per bus, boolean
  // \todo rename flipped z
  uint32_t module_select {0};

  // \todo possibly parametrize all these
  static constexpr uint16_t XinBus{4};
  static constexpr uint16_t WiresInZ{20};
  static constexpr uint16_t GridsInY{80};
  static constexpr uint16_t maxChannel{120}; // \todo parameter

public:

  void swap_on(bool swap) {
    swap_wires_on = swap;
  }

  /** @brief select hardware configuration (module) of the detector for correct wire swapping */
  void select_module(uint32_t module) {
    module_select = module;
  }

  /** @brief identifies which channels are wires, from drawing by Anton */
  bool isWire(uint16_t channel) override { return (channel < GridsInY); }

  /** @brief identifies which channels are grids, from drawing by Anton */
  bool isGrid(uint16_t channel) override { return (channel >= GridsInY) && (channel < maxChannel); }

  /** @brief return the x coordinate of the detector */
  uint32_t x(uint8_t bus, uint16_t channel) override {
    if (swap_wires_on)
      swap(channel);
    // wire == channel, wires range from 0 - 79
    return channel / WiresInZ + bus * XinBus;
  }

  /** @brief return the y coordinate of the detector */
  uint32_t y(uint8_t bus, uint16_t channel) override {
    (void) bus; //unused
    return channel - GridsInY; // ycoords range from 0 to 47
  }

  /** @brief return the z coordinate of the detector */
  uint32_t z(uint8_t bus, uint16_t channel) override {
    (void) bus; //unused

    if (swap_wires_on)
      swap(channel);
    if (module_select == 1) {
      return WiresInZ - channel % WiresInZ - (unsigned short)(1);
    } else {
      return channel % WiresInZ;
    }
  }
};
