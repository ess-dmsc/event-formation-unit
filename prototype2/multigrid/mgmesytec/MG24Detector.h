/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multigrid electronics Handles mappings between (digitizere, channels) and (x,y,z) coordinates
///
/// This currently (18/1 2018) is compatible with MG.24, MG.24.T and MG.CNCS
/// detector demonstrators although not all channels may be in use
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class MG24Detector {
private:
  uint32_t module_select = 0; /// 1 == 20 wires, 0 == 16 in z

public:

  /// \brief select hardware configuration (module) of the detector for correct wire swapping
  void select_module(uint32_t module) {
    XTRACE(DATA, ALW, "Select detector module: %d", module);
    module_select = module;
  }
  /// \brief identifies which channels are wires, from drawing by Anton
  inline bool isWire(int channel) { return (channel <= 79) && (channel >= 0); }

  /// \brief identifies which channels are grids, from drawing by Anton
  inline bool isGrid(int channel) { return (channel >= 80) && (channel < 128); }

  /// \brief return the x coordinate of the detector
  inline int xcoord(int digno, int channel) {
    if (!isWire(channel)) {
      XTRACE(DATA, WAR, "Getting xcoord() from non wire channel");
      return -1;
    }
    /// wire == channel, wires range from 0 - 79
    if (channel < 64) {
      return (channel) / 16 + digno * 4;
    } else {
      return (channel - 64) / 4 + digno * 4;
    }
  }

  /// \brief return the y coordinate of the detector
  inline int ycoord(int channel) {
    if (channel % 2 == 0) {
      channel += 1;
    } else {
      channel -= 1;
    }
    if (!isGrid(channel)) {
      XTRACE(DATA, WAR, "Getting ycoord() from non grid channel");
      return -1;
    }

    return channel - 80; /// ycoords range from 0 to 47
  }

  /// \brief return the z coordinate of the detector
  inline int zcoord(int channel) {
    if (!isWire(channel)) {
      XTRACE(DATA, WAR, "Getting zcoord() from non wire channel");
      return -1;
    }
    if (module_select == 1) {
      if (channel < 64) {
        return 15 - (channel) % 16;
      } else {
        return 3 - ((channel - 64) % 4) + 16;
      }
    } else {
      if (channel % 2 == 0) {
        channel += 1;
      } else {
        channel -= 1;
      }
      if (channel < 64) {
        return (channel) % 16;
      } else {
        return ((channel - 64) % 4) + 16;
      }
    }
  }
};
