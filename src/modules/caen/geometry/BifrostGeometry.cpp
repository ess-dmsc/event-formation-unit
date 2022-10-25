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

#include <caen/geometry/BifrostGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

bool BifrostGeometry::validateData(DataParser::CaenReadout &Data){
   XTRACE(DATA, DEB, "Ring %u, FEN %u, Tube %u", Data.RingId, Data.FENId,
           Data.TubeId);

    if (Data.RingId > MaxRing) {
      XTRACE(DATA, WAR, "RING %d is incompatible with config", Data.RingId);
      Stats.RingErrors++;
      return false;
    }
    return true;
}

int BifrostGeometry::xOffset(int Ring, int Tube) {
  return Ring * NPos + (Tube % 3) * (NPos / 3);
}

int BifrostGeometry::yOffset(int Tube) {
  int Triplet = Tube / 3;
  return Triplet * 3;
}

int BifrostGeometry::posAlongTube(int AmpA, int AmpB) {
  if (AmpA + AmpB == 0) {
    ///\todo add counter
    return -1;
  }
  return ((NPos - 1) * AmpA) / (AmpA + AmpB);
}

uint32_t BifrostGeometry::calcPixel(DataParser::CaenReadout &Data) {
  int xoff = xOffset(Data.RingId, Data.TubeId);
  int yoff = yOffset(Data.TubeId);
  int xlocal = xCoord(Data.AmpA, Data.AmpB);
  int ylocal = yCoord(Data.AmpA, Data.AmpB);
  uint32_t pixel = ESSGeom->pixel2D(xoff + xlocal, yoff + ylocal);

  XTRACE(DATA, DEB, "xoffset %d, xlocal %d, yoffset %d, ylocal %d, pixel %hu",
         xoff, xlocal, yoff, ylocal, pixel);

  return pixel;
}

} // namespace Caen
