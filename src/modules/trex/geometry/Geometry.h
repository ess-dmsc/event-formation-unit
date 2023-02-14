// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TREX geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>

#include <string>
#include <utility>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Trex {

class Geometry {
public:
  virtual bool isWire(uint8_t LocalHybridID) = 0;
  virtual bool isGrid(uint8_t LocalHybridID) = 0;
  virtual uint16_t xAndzCoord(uint8_t RingID, uint8_t FENID, uint8_t HybridID,
                              uint8_t VMMID, uint8_t Channel, uint16_t XOffset,
                              bool Rotated) = 0;
  virtual uint16_t yCoord(uint8_t HybridID, uint8_t VMMID, uint8_t Channel,
                          uint16_t YOffset, bool Rotated, bool Short) = 0;
  virtual bool validGridMapping(uint8_t HybridID, uint8_t VMMID,
                                uint8_t Channel, bool Short) = 0;
  virtual bool validWireMapping(uint8_t HybridID, uint8_t VMMID,
                                uint8_t Channel) = 0;
  static constexpr uint16_t InvalidCoord = 65535;
};
} // namespace Trex
