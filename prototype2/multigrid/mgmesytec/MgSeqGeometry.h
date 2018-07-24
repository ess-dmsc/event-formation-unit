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

#include <common/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class MgSeqGeometry {
private:
  uint32_t module_select {0}; // 1 == 20 wires, 0 == 16 in z
  bool swap_wires_on {false};

public:

  /** @brief select hardware configuration (module) of the detector for correct wire swapping */
  void select_module(uint32_t module) {
    XTRACE(DATA, ALW, "Select detector module: %d\n", module);
    module_select = module;
  }

  void swap_on(bool swap) {
    swap_wires_on = swap;
  }

  /** @brief identifies which channels are wires, from drawing by Anton */
  inline bool isWire(int channel) { return (channel <= 79) && (channel >= 0); }

  /** @brief identifies which channels are grids, from drawing by Anton */
  inline bool isGrid(int channel) { return (channel >= 80) && (channel < 128); }

  inline void swap_wires(int& channel)
  {
    if (channel % 2 == 0) {
      channel += 1;
    } else {
      channel -= 1;
    }
  }

  /** @brief return the x coordinate of the detector */
  inline int xcoord(int bus, int channel) {
    if (!isWire(channel)) {
      XTRACE(DATA, WAR, "Getting xcoord() from non wire channel\n");
      return -1;
    }
    if (swap_wires_on)
      swap_wires(channel);
    // wire == channel, wires range from 0 - 79
    return (channel) / 20 + bus * 4;
  }

  /** @brief return the y coordinate of the detector */
  inline int ycoord(int channel) {
    if (!isGrid(channel)) {
      XTRACE(DATA, WAR, "Getting ycoord() from non grid channel\n");
      return -1;
    }

    return channel - 80; // ycoords range from 0 to 47
  }

  /** @brief return the z coordinate of the detector */
  inline int zcoord(int channel) {
    if (!isWire(channel)) {
      XTRACE(DATA, WAR, "Getting zcoord() from non wire channel\n");
      return -1;
    }
    if (swap_wires_on)
      swap_wires(channel);
    if (module_select == 1) {
      return 19 - (channel) % 20;
    } else {
      return (channel) % 20;
    }
  }
};
