// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for Bifrost:
/// https://project.esss.dk/owncloud/index.php/s/AMKp67jcTGmCFmt
///
//===----------------------------------------------------------------------===//

#include <modules/bifrost/geometry/BifrostGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

BifrostGeometry::BifrostGeometry(Config &CaenConfiguration) {
  ESSGeom = new ESSGeometry(900, 15, 1, 1);
  setResolution(CaenConfiguration.Resolution);
  MaxRing = CaenConfiguration.MaxRing;
  MaxFEN = CaenConfiguration.MaxFEN;
  MaxTube = CaenConfiguration.MaxTube;
}

bool BifrostGeometry::validateData(DataParser::CaenReadout &Data) {
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

int BifrostGeometry::xOffset(int Ring, int TubeId) {
  int RingOffset = Ring * NPos;
  int TubeOffset = (TubeId % 3) * (NPos / 3);
  XTRACE(DATA, DEB, "RingOffset %d, TubeOffset %d", RingOffset, TubeOffset);
  return RingOffset + TubeOffset;
}

int BifrostGeometry::yOffset(int TubeId) {
  int Triplet = TubeId / 3;
  return Triplet * 3;
}


std::pair<int, float> BifrostGeometry::calcTubeAndPos(int AmpA, int AmpB) {
  if (AmpA + AmpB == 0) {
    XTRACE(DATA, DEB, "Sum of amplitudes is 0");
    (*Stats.AmplitudeZero)++;
    return std::pair(-1, -1.0);
  }

  float GlobalPos = 1.0 * AmpA / (AmpA + AmpB); // [0.0 ; 1.0]
  int i;
  float Upper;
  float Lower;
  for (i = 0; i < 6; i+=2) {
    Lower = CaenCalibration.BifrostCalibration.Calib[i];
    Upper = CaenCalibration.BifrostCalibration.Calib[i+1];
    if ((GlobalPos >= Lower) and (GlobalPos <= Upper)) {
      break;
    }
  }
  if (i == 6) {
    XTRACE(DATA, DEB, "A %d, B %d, GlobalPos %f outside valid region",
           AmpA, AmpB, GlobalPos);
    (*Stats.OutsideTube)++;
    return std::pair(-1, -1.0);
  }
  float NormPos = (GlobalPos - Lower)/(Upper - Lower);
  XTRACE(DATA, DEB, "interval %d, GlobalPos %f, NormPos %f", i/2, GlobalPos, NormPos);
  return std::make_pair(i/2, NormPos);
}


uint32_t BifrostGeometry::calcPixel(DataParser::CaenReadout &Data) {
  int xoff = xOffset(Data.RingId, Data.TubeId);
  int yoff = yOffset(Data.TubeId);

  std::pair<int, float> TubePos = calcTubeAndPos(Data.AmpA, Data.AmpB);
  int ylocal = TubePos.first;
  int xlocal = TubePos.second * (TubePixellation - 1);

  uint32_t pixel = ESSGeom->pixel2D(xoff + xlocal, yoff + ylocal);
  if (pixel == 0) {
    XTRACE(DATA, WAR, "xoffset %d, xlocal %d, yoffset %d, ylocal %d, pixel %hu",
           xoff, xlocal, yoff, ylocal, pixel);
  } else {
  XTRACE(DATA, DEB, "xoffset %d, xlocal %d, yoffset %d, ylocal %d, pixel %hu",
         xoff, xlocal, yoff, ylocal, pixel);
  }

  return pixel;
}

} // namespace Caen
