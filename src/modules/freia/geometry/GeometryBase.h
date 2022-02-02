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

  virtual uint16_t yCoord(uint16_t YOffset, uint8_t VMM, uint8_t Channel) = 0;

  static const uint16_t InvalidCoord;
  static const uint16_t NumStrips;
  static const uint16_t NumWires;
  static const uint16_t MinWireChannel;
  static const uint16_t MaxWireChannel;
};
inline uint16_t const GeometryBase::InvalidCoord = 0xFFFF;
inline uint16_t const GeometryBase::NumStrips = 64;
inline uint16_t const GeometryBase::NumWires = 32;
inline uint16_t const GeometryBase::MinWireChannel = 16;
inline uint16_t const GeometryBase::MaxWireChannel = 47;
} // namespace Freia
