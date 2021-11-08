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
#include <freia/geometry/GeometryBase.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class AMORGeometry : public GeometryBase {
public:
  ///\brief return global x-coordinate from the digital geometry
  /// Formulae taken from the Freia ICD, AMOR section
  /// strip = 64 - channel
  /// x = strip - 1       ( == 63 - channel )
  uint16_t xCoord(uint8_t VMM, uint8_t Channel) {
    if (Channel >= NumStrips) {
      XTRACE(DATA, WAR, "Invalid Channel %d (Max %d)", Channel, NumStrips - 1);
      return InvalidCoord;
    }

    if (not isXCoord(VMM)) {
      XTRACE(DATA, WAR, "Invalid VMM (%d) for x-coordinates", VMM);
      return InvalidCoord;
    } else {
      return 63 - Channel;
    }
  }

  ///\brief return global y-coordinate from the digital geometry
  /// Formulae taken from the Freia ICD, AMOR section
  /// wire = channel + 1 - 16
  /// y = (cass - 1) * 32 + 47 - channel
  uint16_t yCoord(uint8_t Cassette, uint8_t VMM, uint8_t Channel) {
    if (Cassette == 0) {
      XTRACE(DATA, WAR, "Cassette 0 is not valid");
      return InvalidCoord;
    }

    if ((Channel < MinWireChannel) or (Channel > MaxWireChannel)) {
      XTRACE(DATA, WAR, "Invalid Channel %d (%d <= ch <= %d)",
             Channel, MinWireChannel, MaxWireChannel);
      return InvalidCoord;
    }

    if (not isYCoord(VMM)) {
      XTRACE(DATA, WAR, "Invalid VMM (%d) for y-coordinates", VMM);
      return InvalidCoord;
    } else {
      return (Cassette - 1) * NumWires + MaxWireChannel - Channel;
    }
  }


  // x-coordinates are strips, which are on VMM 0
  bool isXCoord(uint8_t VMM) {
    return not isYCoord(VMM);
  }

  // y-coordinates are wires, which are on VMM 1
  bool isYCoord(uint8_t VMM) {
    return (VMM & 0x1);
  }

  // Return the local cassette
  uint8_t cassette(uint8_t FEN, uint8_t VMM) {
    return 2 * (FEN - 1) + ((VMM & 0x0f) >> 1);
  }
};

} // namespace
