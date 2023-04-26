// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Attempt to handle the complex DREAM geometry which is assembled
/// from five different CDT module types with different characeteristics
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <dream/geometry/MagicGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

int MagicGeometry::getPixel(Config::ModuleParms &Parms,
                          DataParser::DreamReadout &Data) {

  int Pixel{0};
  XTRACE(DATA, DEB, "Type: %u", Parms.Type);

  switch (Parms.Type) {
  case Config::MagicB:
    Pixel = magicb.getPixelId(Parms, Data);
    break;

  case Config::Mantle:
    Pixel = mantle.getPixelId(Parms, Data);
    break;

  default:
    XTRACE(DATA, WAR, "Unknown detector");
    break;
  }
  int GlobalPixel = getPixelOffset(Parms.Type) + Pixel;
  XTRACE(DATA, DEB, "Local Pixel: %d, Global Pixel: %d", Pixel, GlobalPixel);
  return GlobalPixel;
}

int MagicGeometry::getPixelOffset(Config::ModuleType Type) {
  int RetVal{-1};
  switch (Type) {
  case Config::MagicB:
    RetVal = 0;
    break;
  case Config::Mantle:
    RetVal = 229376;
    break;
  default:
    XTRACE(DATA, WAR, "Module type not valid for MAGIC");
    break;
  }
  return RetVal;
  ;
}

} // namespace Dream
