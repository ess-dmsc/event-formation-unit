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

#include <multigrid/mgmesytec/BusGeometry.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

class MG24Geometry : public BusGeometry {

public:

  /** @brief returns wire */
  uint32_t x(uint16_t channel) const override {
    if (swap_wires_) {
      swap(channel);
    }
    if (channel < 64) {
      return channel / max_z_;
    } else {
      return (channel - 64) * 4 / max_z_;
    }
    // does not respect x-flipping
  }
};

class MG24GeometryA : public MG24Geometry {
public:

  /** \brief return the z coordinate of the detector */
  uint32_t z(uint16_t channel) const override {
    if (swap_wires_) {
      swap(channel);
    }
    if (channel < 64) {
      return (channel) % max_z_;
    } else {
      return ((channel - 64) % 4) + max_z_;
    }
  }
};

// Equivalent of what was previously module_select
class MG24GeometryB : public MG24Geometry {
public:

  /** \brief return the z coordinate of the detector */
  uint32_t z(uint16_t channel) const override {
    if (swap_wires_) {
      swap(channel);
    }
    if (channel < 64) {
      return max_z_ - 1 - (channel) % max_z_;
    } else {
      return 3 - ((channel - 64) % 4) + max_z_;
    }
  }
};


}