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

class MG24GeometryA : public MGSeqGeometry {
public:
  uint16_t wire(uint8_t VMM, uint16_t channel) const override {
    if (channel < 16) {
      return MGSeqGeometry::wire(VMM, channel);
    } else if (channel < 32){
      return MGSeqGeometry::wire(VMM, channel + 4);
    } else if (channel < 48){
      return MGSeqGeometry::wire(VMM, channel + 8);
    } else if (channel < 64){
      return MGSeqGeometry::wire(VMM, channel + 12);
    } else if (channel < 68){
      return MGSeqGeometry::wire(VMM, channel - 48);
    } else if (channel < 72){
      return MGSeqGeometry::wire(VMM, channel - 32);
    } else if (channel < 76){
      return MGSeqGeometry::wire(VMM, channel - 16);
    } else {
      return MGSeqGeometry::wire(VMM, channel);
    }
  }
};

// Equivalent of what was previously module_select
class MG24GeometryB : public MGSeqGeometry {
public:
  /** \brief return the z coordinate of the detector */

  // \todo this is wrong, need to draw a diagram for this module like for SRS tests
  uint16_t wire(uint8_t VMM, uint16_t channel) const override {
    if (channel < 16) {
      return MGSeqGeometry::wire(VMM, channel);
    } else if (channel < 32){
      return MGSeqGeometry::wire(VMM, channel + 4);
    } else if (channel < 48){
      return MGSeqGeometry::wire(VMM, channel + 8);
    } else if (channel < 64){
      return MGSeqGeometry::wire(VMM, channel + 12);
    } else if (channel < 68){
      return MGSeqGeometry::wire(VMM, channel - 48);
    } else if (channel < 72){
      return MGSeqGeometry::wire(VMM, channel - 32);
    } else if (channel < 76){
      return MGSeqGeometry::wire(VMM, channel - 16);
    } else {
      return MGSeqGeometry::wire(VMM, channel);
    }
  }
};


}