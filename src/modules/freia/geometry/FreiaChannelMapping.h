// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include<cinttypes>
#include <common/debug/Trace.h>
#include <freia/geometry/GeometryBase.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class FreiaGeometry : public GeometryBase {
public:
  ///\brief return global x-coordinate from the digital geometry
  /// Formulae taken from the Freia ICD
  /// strip = 64 - channel
  /// x = strip - 1
  uint16_t xCoord(uint16_t __attribute__((unused)) XOffset, uint8_t VMM, uint8_t Channel) {
    if (Channel >= NumStrips) {
      XTRACE(DATA, WAR, "Invalid Channel %d (Max %d)", Channel, NumStrips - 1);
      return GeometryBase::InvalidCoord;
    }

    if (not isXCoord(VMM)) {
      XTRACE(DATA, WAR, "Invalid VMM (%d) for x-coordinates", VMM);
      return GeometryBase::InvalidCoord;
    } else {
      return NumStrips - Channel - 1;
    }
  }

  ///\brief return global y-coordinate from the digital geometry
  /// Formulae taken from the Freia ICD
  /// wire = 32 - (channel - 15)
  /// y = cass * 32 + 32 - wire (= cass * 32 + 47 - channel)
  uint16_t yCoord(uint16_t YOffset, uint8_t VMM, uint8_t Channel) {
    XTRACE(DATA, DEB, "YOffset %u, VMM %u, Channel %u", YOffset, VMM, Channel);

    if ((Channel < MinWireChannel) or (Channel > MaxWireChannel)) {
      XTRACE(DATA, WAR, "Invalid Channel %d (%d <= ch <= %d)", Channel,
             MinWireChannel, MaxWireChannel);
      return GeometryBase::InvalidCoord;
    }

    if (not isYCoord(VMM)) {
      XTRACE(DATA, WAR, "Invalid VMM (%d) for y-coordinates", VMM);
      return GeometryBase::InvalidCoord;
    } else {
      return YOffset + MaxWireChannel - Channel;
    }
  }

  // x-coordinates are strips, which are on VMM 1
  bool isXCoord(uint8_t VMM) { return (VMM & 0x1); }

  // y-coordinates are wires, which are on VMM 0
  bool isYCoord(uint8_t VMM) { return not isXCoord(VMM); }
};

} // namespace Freia
