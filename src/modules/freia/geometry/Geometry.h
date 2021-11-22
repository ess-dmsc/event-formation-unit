// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multiblade geometry selector class
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <freia/geometry/AMORChannelMapping.h>
#include <freia/geometry/FreiaChannelMapping.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class Geometry {
public:
  Geometry() {
    GeometryInst = &FreiaGeom;
  }

  bool setGeometry(std::string NewGeometry) {
    if (NewGeometry == "AMOR") {
      GeometryInst = &AMORGeom;
      return true;
    }
    if (NewGeometry == "Freia") {
      GeometryInst = &FreiaGeom;
      return true;
    }
    XTRACE(DATA, ERR, "Unknown instrument mapping: %s", NewGeometry.c_str());
    return false;
  }

  // wrapper function for specific instrument geometry instance
  uint16_t xCoord(uint8_t VMM, uint8_t Channel) {
    return GeometryInst->xCoord(VMM, Channel);
  }

  // wrapper function for specific instrument geometry instance
  uint16_t yCoord(uint8_t Cassette, uint8_t VMM, uint8_t Channel) {
    return GeometryInst->yCoord(Cassette, VMM, Channel);
  }

  // wrapper function for specific instrument geometry instance
  bool isXCoord(uint8_t VMM) { return GeometryInst->isXCoord(VMM); }

  // wrapper function for specific instrument geometry instance
  bool isYCoord(uint8_t VMM) { return GeometryInst->isYCoord(VMM); }

  /// \todo hack because of lack of header only static consts
  uint16_t getNumWires() { return GeometryInst->NumWires; }
  uint16_t getNumStrips() { return GeometryInst->NumStrips; }

private:
  AMORGeometry AMORGeom;
  FreiaGeometry FreiaGeom;
  GeometryBase * GeometryInst{nullptr};
};
}
