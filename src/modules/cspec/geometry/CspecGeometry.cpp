// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for CSPEC:
/// https://project.esss.dk/owncloud/index.php/s/3i0RtWiVwNM6EBY (v1 -- out of
/// date as of 2023-02)
///
//===----------------------------------------------------------------------===//

#include <modules/cspec/geometry/CspecGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

CspecGeometry::CspecGeometry(Config &CaenConfiguration) {
  ESSGeom = new ESSGeometry(900, 180, 1, 1);
  setResolution(CaenConfiguration.Resolution);
  MaxRing = CaenConfiguration.MaxRing;
  MaxFEN = CaenConfiguration.MaxFEN;
  MaxGroup = CaenConfiguration.MaxGroup;
}

bool CspecGeometry::validateData(DataParser::CaenReadout &Data) {
  int Ring = Data.FiberId / 2;
  XTRACE(DATA, DEB, "FiberId: %u, Ring %d, FEN %u, Group %u", Data.FiberId, Ring,
      Data.FENId, Data.Group);

  if (Ring > MaxRing) {
    XTRACE(DATA, WAR, "RING %d is incompatible with config", Ring);
    Stats.RingErrors++;
    return false;
  }

  if (Data.FENId > MaxFEN) {
    XTRACE(DATA, WAR, "FEN %d is incompatible with config", Data.FENId);
    Stats.FENErrors++;
    return false;
  }

  if (Data.Group > MaxGroup) {
    XTRACE(DATA, WAR, "Group %d is incompatible with config", Data.Group);
    Stats.GroupErrors++;
    return false;
  }
  return true;
}

int CspecGeometry::xOffset(int Ring, int Group) {
  ///\todo Determine the 'real' x-offset once a new ICD is decided for 3He CSPEC
  return Ring * NPos + (Group % 24) * (NPos / 24);
}


int CspecGeometry::posAlongUnit(int AmpA, int AmpB) {
  if (AmpA + AmpB == 0) {
    ///\todo add counter
    return -1;
  }
  return ((NPos - 1) * AmpA) / (AmpA + AmpB);
}

uint32_t CspecGeometry::calcPixel(DataParser::CaenReadout &Data) {
  int Ring = Data.FiberId / 2;
  int xoff = xOffset(Ring, Data.Group);
  int ylocal = yCoord(Data.AmpA, Data.AmpB);
  uint32_t pixel = ESSGeom->pixel2D(xoff, ylocal);

  XTRACE(DATA, DEB, "xoffset %d, ylocal %d, pixel %hu", xoff, ylocal, pixel);

  return pixel;
}

} // namespace Caen
