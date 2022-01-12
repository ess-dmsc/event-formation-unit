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
  virtual std::pair<uint8_t, uint8_t> xAndzCoord(uint8_t RingID, uint8_t FENID, uint8_t HybridID, uint8_t VMMID, uint8_t Channel) = 0;
  virtual uint16_t yCoord(uint8_t HybridID, uint8_t VMMID, uint8_t Channel) = 0;
  static const uint16_t InvalidCoord;
};
inline uint16_t const GeometryBase::InvalidCoord = 0xFFFF;
}
