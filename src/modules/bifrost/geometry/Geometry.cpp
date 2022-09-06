// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for Bifrost:
/// https://project.esss.dk/owncloud/index.php/s/AMKp67jcTGmCFmt
///
//===----------------------------------------------------------------------===//

#include <bifrost/geometry/Geometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Bifrost {

int Geometry::xOffset(int Ring, int Tube) {
  return Ring * PosResolution + (Tube % 3) * (PosResolution / 3);
}

int Geometry::yOffset(int Tube) {
  int Triplet = Tube / 3;
  return Triplet * 3;
}

int Geometry::posAlongTube(int AmpA, int AmpB) {
  if (AmpA + AmpB == 0) {
    ///\todo add counter
    return -1;
  }
  return ((PosResolution - 1) * AmpA) / (AmpA + AmpB);
}

} // namespace Bifrost
