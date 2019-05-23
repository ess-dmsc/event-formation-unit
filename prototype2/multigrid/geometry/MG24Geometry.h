/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Handles mappings between (digitizers, channels) and (x,y,z) coordinates
///
/// This currently (18/1 2018) is compatible with MG.24, MG.24.T and MG.CNCS
/// detector demonstrators although not all channels may be in use
///
//===----------------------------------------------------------------------===//

#pragma once

#include <multigrid/geometry/MGSeqGeometry.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

// \todo this requires more work, very likely need to reimplement wire()
//        and must test that reverse mapping from global wire works

class MG24Geometry : public MGSeqGeometry {

public:

  /** @brief returns wire */
  uint32_t x_from_wire(uint16_t w) const override {
    uint32_t ret;
    if (w < 64) {
      ret = w / max_z();
    } else {
      ret = (w - 64) * 4 / max_z();
    }
    return flipped_x() ? (max_x() - 1u - ret) : ret;
  }
};

class MG24GeometryA : public MG24Geometry {
public:

  /** \brief return the z coordinate of the detector */
  uint32_t z_from_wire(uint16_t w) const override {
    uint32_t ret;
    if (w < 64) {
      ret = w % max_z();
    } else {
      ret = ((w - 64) % 4) + max_z();
    }
    return flipped_z() ? (max_z() - 1u - ret) : ret;
  }
};

// Equivalent of what was previously module_select
class MG24GeometryB : public MG24Geometry {
public:

  /** \brief return the z coordinate of the detector */
  uint32_t z_from_wire(uint16_t w) const override {
    uint32_t ret;
    if (w < 64) {
      ret = max_z() - 1u - (w % max_z());
    } else {
      ret = 3 - ((w - 64) % 4) + max_z();
    }
    return flipped_z() ? (max_z() - 1u - ret) : ret;
  }
};


}