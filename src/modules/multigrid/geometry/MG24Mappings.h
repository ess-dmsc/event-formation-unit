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

#include <multigrid/geometry/MGSeqMappings.h>

#include <common/debug/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

class MG24MappingsA : public MGSeqMappings {
public:
  uint16_t wire(uint16_t channel) const override {
    if (channel < 16) {
      return MGSeqMappings::wire(channel);
    } else if (channel < 32){
      return MGSeqMappings::wire(channel + 4);
    } else if (channel < 48){
      return MGSeqMappings::wire(channel + 8);
    } else if (channel < 64){
      return MGSeqMappings::wire(channel + 12);
    } else if (channel < 68){
      return MGSeqMappings::wire(channel - 48);
    } else if (channel < 72){
      return MGSeqMappings::wire(channel - 32);
    } else if (channel < 76){
      return MGSeqMappings::wire(channel - 16);
    } else {
      return MGSeqMappings::wire(channel);
    }
  }

  std::string debug(std::string prefix) const override {
    return "[MG24MappingsA]/" + MGSeqMappings::debug(prefix);
  }
};

// Equivalent of what was previously module_select
class MG24MappingsB : public MGSeqMappings {
public:
  /** \brief return the z coordinate of the detector */

  // \todo this is wrong, need to draw a diagram for this module like for SRS tests
  uint16_t wire(uint16_t channel) const override {
    if (channel < 16) {
      return MGSeqMappings::wire(channel);
    } else if (channel < 32){
      return MGSeqMappings::wire(channel + 4);
    } else if (channel < 48){
      return MGSeqMappings::wire(channel + 8);
    } else if (channel < 64){
      return MGSeqMappings::wire(channel + 12);
    } else if (channel < 68){
      return MGSeqMappings::wire(channel - 48);
    } else if (channel < 72){
      return MGSeqMappings::wire(channel - 32);
    } else if (channel < 76){
      return MGSeqMappings::wire(channel - 16);
    } else {
      return MGSeqMappings::wire(channel);
    }
  }

  std::string debug(std::string prefix) const override {
    return "[MG24MappingsB]/" + MGSeqMappings::debug(prefix);
  }
};


}