// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for Miracles:
/// https://project.esss.dk/owncloud/index.php/s/AMKp67jcTGmCFmt
///
//===----------------------------------------------------------------------===//

#include <modules/caen/geometry/MiraclesGeometry.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Caen {

uint32_t MiraclesGeometry::calcPixel(DataParser::CaenReadout &Data) {
  int x = xCoord(Data.RingId, Data.TubeId, Data.AmpA, Data.AmpB);
  int y = yCoord(Data.RingId, Data.AmpA, Data.AmpB);
  uint32_t pixel = ESSGeom->pixel2D(x, y);

  XTRACE(DATA, DEB, "xcoord %d, ycoord %d, pixel %hu",
         x, y, pixel);

  return pixel;
}

bool MiraclesGeometry::validateData(DataParser::CaenReadout &Data){
  XTRACE(DATA, DEB, "todo, implement validate data. %u", Data.RingId);
  return true;
}

int MiraclesGeometry::xCoord(int Ring, int Tube, int AmpA, int AmpB) {
  int xOffset = 2 * Tube;
  if ((Ring == 1) or (Ring == 3)) {
    xOffset+=24;
  }
  return xOffset + tubeAorB(AmpA, AmpB);
}

int MiraclesGeometry::yCoord(int Ring, int AmpA, int AmpB) {
  int offset{0};
  if ((Ring == 2) or (Ring == 3)) {
    offset += 100;
  }
  return offset + posAlongTube(AmpA, AmpB);
}


// 0 is A, 1 is B
int MiraclesGeometry::tubeAorB(int AmpA, int AmpB) {
  float UnitPos = 1.0 * AmpA /(AmpA + AmpB);
  if (UnitPos < 0.5) {
    XTRACE(DATA, DEB, "A-tube (pos %f)", UnitPos);
    return 0;
  } else {
    XTRACE(DATA, DEB, "B-tube (pos %f)", UnitPos);
    return 1;
  }
}

int MiraclesGeometry::posAlongTube(int AmpA, int AmpB) {
  if (AmpA + AmpB == 0) {
    ///\todo add counter
    return -1;
  }

  int pos = (1.0 * AmpA /(AmpA + AmpB))*NPos;
  XTRACE(DATA, DEB, "Position along tube pair %d", pos);

  if (tubeAorB(AmpA, AmpB) == 0) {
    return pos;
  } else {
    return 64 - (pos - 64);
  }
}

} // namespace Caen
