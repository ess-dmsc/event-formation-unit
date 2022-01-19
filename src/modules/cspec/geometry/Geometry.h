// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <string>
#include <utility>


// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Cspec {

class Geometry {
public:
  virtual bool isWire(uint8_t LocalHybridID) = 0;
  virtual bool isGrid(uint8_t LocalHybridID) = 0;
  virtual uint16_t xAndzCoord(uint8_t FENID, uint8_t HybridID, uint8_t VMMID, uint8_t Channel, uint16_t XOffset, bool Rotated) = 0;
  virtual uint8_t yCoord(uint8_t HybridID, uint8_t VMMID, uint8_t Channel, uint16_t YOffset, bool Rotated, bool Short) = 0;
  static const uint8_t InvalidCoord;
};
inline uint8_t const InvalidCoord = 255;
}
