// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Attempt to handle the complex DREAM geometry which is assembled
/// from five different CDT module types with different characeteristics
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <dream/geometry/DreamGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

bool DreamGeometry::validateReadoutData(
    const DataParser::CDTReadout &Data) const {
  int Ring = Data.FiberId / 2;

  return validateAll(
      [&]() { return validateRing(Ring); },
      [&]() { return validateFEN(Data.FENId); },
      [&]() { return validateConfigMapping(Ring, Data.FENId); });
}

uint32_t DreamGeometry::calcPixelImpl(const DataParser::CDTReadout &Data) const {
  int Ring = Data.FiberId / 2;
  const Config::ModuleParms &Parms = getModuleParms(Ring, Data.FENId);

  int Pixel{0};
  XTRACE(DATA, DEB, "Type: %u", Parms.Type);

  switch (Parms.Type) {
  case Config::BwEndCap:
    Pixel = bwec.calcPixelId(Parms, Data);
    break;

  case Config::FwEndCap:
    Pixel = fwec.calcPixelId(Parms, Data);
    break;

  case Config::DreamMantle:
    Pixel = mantle.calcPixelId(Parms, Data);
    break;

  case Config::HR: // fallthrough \todo might or might not work
  case Config::SANS:
    Pixel = cuboid.calcPixelId(Parms, Data);
    break;
  default:
    XTRACE(DATA, WAR, "Unknown detector");
    break;
  }

  if (Pixel == 0) {
    XTRACE(DATA, WAR, "Invalid pixel returned in module: %i", Parms.Type);
    return 0;
  }

  int Offset = getPixelOffset(Parms.Type);
  if (Offset == -1) {
    XTRACE(DATA, WAR, "No offset given for module %i", Parms.Type);
    return 0;
  }
  int GlobalPixel = Offset + Pixel;
  XTRACE(DATA, DEB, "Local Pixel: %d, Global Pixel: %d", Pixel, GlobalPixel);
  return GlobalPixel;
}

int DreamGeometry::getPixelOffset(Config::ModuleType Type) const {
  int RetVal{-1};
  switch (Type) {
  case Config::FwEndCap:
    RetVal = 0;
    break;
  case Config::BwEndCap:
    RetVal = 71680; ///< Offset value from ICD
    break;
  case Config::DreamMantle:
    RetVal = 229376; ///< Offset value from ICD
    break;
  case Config::SANS:
    RetVal = 720896; ///< Offset value from ICD
    break;
  case Config::HR:
    RetVal = 1122304; ///< Offset value from ICD
    break;
  default:
    XTRACE(DATA, WAR, "Module type not valid for DREAM");
    break;
  }
  return RetVal;
}

} // namespace Dream
