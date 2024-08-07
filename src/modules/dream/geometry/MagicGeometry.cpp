// Copyright (C) 2023 - 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Handle MAGIC geometry similarly as DREAM
///
/// Uses the formulae and parameters from the MAGIC ICD which can be
/// located through
/// https://confluence.esss.lu.se/display/ECDC/Instrument+Status+Overview
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <dream/geometry/MagicGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

int MagicGeometry::getPixel(Config::ModuleParms &Parms,
                            DataParser::CDTReadout &Data) {

  int Pixel{0};
  XTRACE(DATA, DEB, "Type: %u", Parms.Type);

  switch (Parms.Type) {
  case Config::PA:
    Pixel = padetector.getPixelId(Parms, Data);
    break;

  case Config::FR:
    Pixel = frdetector.getPixelId(Parms, Data);
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

///\brief the pixel offset values are defined in the MAGIC ICD
int MagicGeometry::getPixelOffset(Config::ModuleType Type) {
  int RetVal{-1};
  switch (Type) {
  case Config::FR:
    RetVal = 0;
    break;
  case Config::PA:
    RetVal = 245760;
    break;
  default:
    XTRACE(DATA, WAR, "Module type not valid for MAGIC");
    break;
  }
  return RetVal;
}

} // namespace Dream
