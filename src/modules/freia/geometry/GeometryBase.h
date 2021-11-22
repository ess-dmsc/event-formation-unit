// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class GeometryBase {
public:
  virtual bool isXCoord(uint8_t VMM) = 0;

  virtual bool isYCoord(uint8_t VMM) = 0;

  virtual uint16_t xCoord(uint8_t VMM, uint8_t Channel) = 0;

  virtual uint16_t yCoord(uint8_t Cassette, uint8_t VMM, uint8_t Channel) = 0;

  constexpr static uint16_t InvalidCoord{0xFFFF};
  constexpr static uint16_t NumStrips{64};
  constexpr static uint16_t NumWires{32};
  constexpr static uint16_t MinWireChannel{16};
  constexpr static uint16_t MaxWireChannel{47};
};
}
