// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Heimdal geometry class
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <dream/geometry/HeimdalGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

int HeimdalGeometry::getPixel(Config::ModuleParms &Parms,
                            DataParser::DreamReadout &Data) {

  int Pixel{0};
  XTRACE(DATA, DEB, "Type: %u", Parms.Type);

  switch (Parms.Type) {
    case Config::HeimdalMantle:
      Pixel = mantle.getPixelId(Parms, Data);
      break;
    default:
      XTRACE(DATA, WAR, "Unknown detector");
      break;
  }

  int Offset = getPixelOffset(Parms.Type);
  if (Offset == -1) {
    return 0;
  }
  int GlobalPixel = Offset + Pixel;
  XTRACE(DATA, DEB, "Local Pixel: %d, Global Pixel: %d", Pixel, GlobalPixel);
  return GlobalPixel;
}

int HeimdalGeometry::getPixelOffset(Config::ModuleType Type) {
  int RetVal{-1};
  switch (Type) {
  case Config::HeimdalMantle:
    RetVal = 0;
    break;
  default:
    XTRACE(DATA, WAR, "Module type %d not valid for HEIMDAL", Type);
    break;
  }
  return RetVal;
}

} // namespace Dream
