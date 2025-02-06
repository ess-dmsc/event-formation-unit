// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
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
#include <freia/geometry/EstiaChannelMapping.h>
#include <freia/geometry/FreiaChannelMapping.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Freia {

class Geometry {
public:
  Geometry() { GeometryInst = &FreiaGeom; }

  bool setGeometry(const std::string &NewGeometry) {
    if (NewGeometry == "AMOR") {
      GeometryInst = &AMORGeom;
      return true;
    }
    if (NewGeometry == "Freia") {
      GeometryInst = &FreiaGeom;
      return true;
    }
    if (NewGeometry == "Estia") {
      GeometryInst = &EstiaGeom;
      return true;
    }
    XTRACE(DATA, ERR, "Unknown instrument mapping: %s", NewGeometry.c_str());
    return false;
  }

  // wrapper function for specific instrument geometry instance
  uint16_t xCoord(uint16_t XOffset, uint8_t VMM, uint8_t Channel) {
    return GeometryInst->xCoord(XOffset, VMM, Channel);
  }

  // wrapper function for specific instrument geometry instance
  uint16_t yCoord(uint16_t YOffset, uint8_t VMM, uint8_t Channel) {
    return GeometryInst->yCoord(YOffset, VMM, Channel);
  }

  // wrapper function for specific instrument geometry instance
  bool isXCoord(uint8_t VMM) { return GeometryInst->isXCoord(VMM); }

  // wrapper function for specific instrument geometry instance
  bool isYCoord(uint8_t VMM) { return GeometryInst->isYCoord(VMM); }

private:
  AMORGeometry AMORGeom;
  FreiaGeometry FreiaGeom;
  EstiaGeometry EstiaGeom;
  GeometryBase *GeometryInst{nullptr};
};
} // namespace Freia
