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

#include <modules/he3cspec/geometry/He3cspecGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

He3cspecGeometry::He3cspecGeometry(Config &CaenConfiguration) {
  ESSGeom = new ESSGeometry(900, 180, 1, 1);
  setResolution(CaenConfiguration.Resolution);
  MaxRing = CaenConfiguration.MaxRing;
  MaxFEN = CaenConfiguration.MaxFEN;
  MaxTube = CaenConfiguration.MaxTube;
}

bool He3cspecGeometry::validateData(DataParser::CaenReadout &Data) {
  XTRACE(DATA, DEB, "Ring %u, FEN %u, Tube %u", Data.RingId, Data.FENId,
         Data.TubeId);

  if (Data.RingId > MaxRing) {
    XTRACE(DATA, WAR, "RING %d is incompatible with config", Data.RingId);
    (*Stats.RingErrors)++;
    return false;
  }

  if (Data.FENId > MaxFEN) {
    XTRACE(DATA, WAR, "FEN %d is incompatible with config", Data.FENId);
    (*Stats.FENErrors)++;
    return false;
  }

  if (Data.TubeId > MaxTube) {
    XTRACE(DATA, WAR, "Tube %d is incompatible with config", Data.TubeId);
    (*Stats.TubeErrors)++;
    return false;
  }
  return true;
}

int He3cspecGeometry::xOffset(int Ring, int Tube) {
  return Ring * NPos + (Tube % 24) * (NPos / 24);
}

int He3cspecGeometry::yOffset(int Tube) {
  int Pack = Tube / 24;
  return Pack * 24;
}

int He3cspecGeometry::posAlongTube(int AmpA, int AmpB) {
  if (AmpA + AmpB == 0) {
    ///\todo add counter
    return -1;
  }
  return ((NPos - 1) * AmpA) / (AmpA + AmpB);
}

uint32_t He3cspecGeometry::calcPixel(DataParser::CaenReadout &Data) {
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
