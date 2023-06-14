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
  MaxTube = CaenConfiguration.MaxTube;
}

bool CspecGeometry::validateData(DataParser::CaenReadout &Data) {
  XTRACE(DATA, DEB, "Ring %u, FEN %u, Tube %u", Data.RingId, Data.FENId,
         Data.TubeId);

  if (Data.RingId > MaxRing) {
    XTRACE(DATA, WAR, "RING %d is incompatible with config", Data.RingId);
    Stats.RingErrors++;
    return false;
  }

  if (Data.FENId > MaxFEN) {
    XTRACE(DATA, WAR, "FEN %d is incompatible with config", Data.FENId);
    Stats.FENErrors++;
    return false;
  }

  if (Data.TubeId > MaxTube) {
    XTRACE(DATA, WAR, "Tube %d is incompatible with config", Data.TubeId);
    Stats.GroupErrors++;
    return false;
  }
  return true;
}

int CspecGeometry::xOffset(int Ring, int Tube) {
  ///\todo Determine the 'real' x-offset once a new ICD is decided for 3He CSPEC
  return Ring * NPos + (Tube % 24) * (NPos / 24);
}

// This is always zero for a cylindrical detector with tubes in a plane, aligned
// along its axis
// int CspecGeometry::yOffset(int Tube) {
//  ///\todo Determine the 'real' y-offset once a new ICD is decided for 3He
//  CSPEC int Pack = Tube / 24; return Pack * 24;
//}

int CspecGeometry::posAlongTube(int AmpA, int AmpB) {
  if (AmpA + AmpB == 0) {
    ///\todo add counter
    return -1;
  }
  return ((NPos - 1) * AmpA) / (AmpA + AmpB);
}

uint32_t CspecGeometry::calcPixel(DataParser::CaenReadout &Data) {
  int xoff = xOffset(Data.RingId, Data.TubeId);
  int ylocal = yCoord(Data.AmpA, Data.AmpB);
  uint32_t pixel = ESSGeom->pixel2D(xoff, ylocal);

  XTRACE(DATA, DEB, "xoffset %d, ylocal %d, pixel %hu", xoff, ylocal, pixel);

  return pixel;
}

} // namespace Caen
