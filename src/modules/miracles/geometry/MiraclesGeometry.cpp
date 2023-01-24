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

#include <cmath>
#include <modules/miracles/geometry/MiraclesGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

MiraclesGeometry::MiraclesGeometry(Config &CaenConfiguration) {
  ///\todo make config dependent
  ESSGeom = new ESSGeometry(48, 128, 1, 1);
  setResolution(CaenConfiguration.Resolution);
  MaxRing = CaenConfiguration.MaxRing;
}

uint32_t MiraclesGeometry::calcPixel(DataParser::CaenReadout &Data) {
  int x = xCoord(Data.RingId, Data.TubeId, Data.AmpA, Data.AmpB);
  int y = yCoord(Data.RingId, Data.AmpA, Data.AmpB);
  uint32_t pixel = ESSGeom->pixel2D(x, y);

  XTRACE(DATA, DEB, "xcoord %d, ycoord %d, pixel %hu", x, y, pixel);

  return pixel;
}

bool MiraclesGeometry::validateData(DataParser::CaenReadout &Data) {
  XTRACE(DATA, DEB, "Ring %u, FEN %u, Tube %u", Data.RingId, Data.FENId,
         Data.TubeId);

  if (Data.RingId > MaxRing) {
    XTRACE(DATA, WAR, "RING %d is incompatible with config", Data.RingId);
    (*Stats.RingErrors)++;
    return false;
  }
  return true;
}

int MiraclesGeometry::xCoord(int Ring, int Tube, int AmpA, int AmpB) {
  int xOffset = 2 * Tube;
  if ((Ring == 1) or (Ring == 3)) {
    xOffset += 24; ///\todo make config dependent
  }
  return xOffset + tubeAorB(AmpA, AmpB);
}

int MiraclesGeometry::yCoord(int Ring, int AmpA, int AmpB) {
  XTRACE(DATA, DEB, "Calculating yCoord, Ring: %u, AmpA: %u, AmpB: %u", Ring,
         AmpA, AmpB);
  int offset{0};
  if ((Ring == 2) or (Ring == 3)) {
    offset += NPos / 2;
  }
  return offset + posAlongTube(AmpA, AmpB);
}

// 0 is A, 1 is B
int MiraclesGeometry::tubeAorB(int AmpA, int AmpB) {
  float UnitPos = 1.0 * AmpA / (AmpA + AmpB);
  if (UnitPos <= 0.5) {
    XTRACE(DATA, DEB, "A-tube (pos %f)", UnitPos);
    return 0;
  } else {
    XTRACE(DATA, DEB, "B-tube (pos %f)", UnitPos);
    return 1;
  }
}

int MiraclesGeometry::posAlongTube(int AmpA, int AmpB) {
  int tubepos;
  if (AmpA + AmpB == 0) {
    XTRACE(DATA, WAR, "AmpA + AmpB == 0, invalid amplitudes");
    ///\todo add counter
    return -1;
  }

  float pos = (1.0 * AmpA / (AmpA + AmpB));
  XTRACE(DATA, DEB, "Position along tube pair %f", pos);

  if (tubeAorB(AmpA, AmpB) == 0) {
    tubepos = pos * 2 * (NPos / 2 - 1);
    XTRACE(DATA, DEB, "A: TubePos %u, pos: %f", tubepos, pos);
    return tubepos;
  } else {
    tubepos = round(NPos / 2 - 1 - (pos - 0.5) * 2 * (NPos / 2 - 1));
    XTRACE(DATA, DEB, "B: TubePos %u, pos %f", tubepos, pos);
    return tubepos;
  }
}

} // namespace Caen
