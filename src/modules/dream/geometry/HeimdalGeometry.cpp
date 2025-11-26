// Copyright (C) 2024 - 2025 European Spallation Source, see LICENSE file
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

bool HeimdalGeometry::validateReadoutData(
    const DataParser::CDTReadout &Data) const {
  int Ring = Data.FiberId / 2;

  return validateAll(
      [&]() { return validateRing(Ring); },
      [&]() { return validateFEN(Data.FENId); },
      [&]() { return validateConfigMapping(Ring, Data.FENId); });
}

uint32_t HeimdalGeometry::calcPixelImpl(const DataParser::CDTReadout &Data) const {
  int Ring = Data.FiberId / 2;
  const Config::ModuleParms &Parms = getModuleParms(Ring, Data.FENId);

  int Pixel{0};
  XTRACE(DATA, DEB, "Type: %u", Parms.Type);

  switch (Parms.Type) {
  case Config::HeimdalMantle:
    Pixel = mantle.calcPixelId(Parms, Data);
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

int HeimdalGeometry::getPixelOffset(Config::ModuleType Type) const {
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
