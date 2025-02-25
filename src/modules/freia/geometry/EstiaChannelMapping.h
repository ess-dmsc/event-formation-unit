// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Estia geometry class
///
/// Mapping from digital identifiers to x- and y- coordinates
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <freia/geometry/GeometryBase.h>

#include <cinttypes>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class EstiaGeometry : public GeometryBase {
public:
  ///\brief return global x-coordinate from the digital geometry
  /// Formulae taken from the Estia ICD
  /// wire = channel + 1 - 16
  /// xoffset = (cass%48) * 32
  /// x = xoffset + wire - 1
  uint16_t xCoord(uint16_t XOffset, uint8_t VMM, uint8_t Channel) {

    if ((Channel < MinWireChannel) or (Channel > MaxWireChannel)) {
      XTRACE(DATA, WAR, "Invalid Channel %d (%d <= ch <= %d)", Channel,
             MinWireChannel, MaxWireChannel);
      return InvalidCoord;
    }

    if (not isXCoord(VMM)) {
      XTRACE(DATA, WAR, "Invalid VMM (%d) for x-coordinates", VMM);
      return InvalidCoord;
    } else {
      return XOffset + Channel - MinWireChannel;
    }
  }


  ///\brief return global y-coordinate from the digital geometry
  /// Formulae taken from the Estia ICD
  /// strip = 64 - channel
  /// yoffset = (cass/48) * 64
  /// y = strip - 1       ( == 63 - channel )
  uint16_t yCoord(uint16_t YOffset, uint8_t VMM, uint8_t Channel) {
    XTRACE(DATA, DEB, "YOffset %u, VMM %u, Channel %u", YOffset, VMM, Channel);
    if (Channel >= NumStrips) {
      XTRACE(DATA, WAR, "Invalid Channel %d (Max %d)", Channel, NumStrips - 1);
      return InvalidCoord;
    }

    if (not isYCoord(VMM)) {
      XTRACE(DATA, WAR, "Invalid VMM (%d) for y-coordinates", VMM);
      return InvalidCoord;
    } else {
      return YOffset + NumStrips - 1 - Channel;
    }
  }


  // y-coordinates are strips, which are on VMM 0
  bool isYCoord(uint8_t VMM) { return not isXCoord(VMM); }

  // x-coordinates are wires, which are on VMM 1
  bool isXCoord(uint8_t VMM) { return (VMM & 0x1); }
};

} // namespace Freia
