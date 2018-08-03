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

#include <multigrid/mgmesytec/MgGeometry.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class MG25Geometry : public MgGeometry {
private:
  uint32_t module_select = 0; /// 1 == 20 wires, 0 == 16 in z

  static constexpr uint16_t XinBus{4};

public:

  /// \brief select hardware configuration (module) of the detector for correct wire swapping
  void select_module(uint32_t module) {
    XTRACE(DATA, ALW, "Select detector module: %d", module);
    module_select = module;
  }

  /** \brief return the x coordinate of the detector */
  uint32_t x(uint8_t bus, uint16_t channel) override {
    // wire == channel, wires range from 0 - 79
    if (channel < 64) {
      return (channel) / 16 + bus * XinBus;
    } else {
      return (channel - 64) / 4 + bus * XinBus;
    }
  }

  /** \brief return the y coordinate of the detector */
  uint32_t y(uint8_t bus, uint16_t channel) override {
    (void) bus; //unused
    return grid(channel);
  }

  /** \brief return the z coordinate of the detector */
  uint32_t z(uint8_t bus, uint16_t channel) override {
    (void) bus; //unused

    if (module_select == 1) {
      if (channel < 64) {
        return 15 - (channel) % 16;
      } else {
        return 3 - ((channel - 64) % 4) + 16;
      }
    } else {
      swap(channel);
      if (channel < 64) {
        return (channel) % 16;
      } else {
        return ((channel - 64) % 4) + 16;
      }
    }
  }
};
