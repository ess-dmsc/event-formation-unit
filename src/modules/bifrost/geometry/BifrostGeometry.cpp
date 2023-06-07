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
  int TubeOffset = (TubeId % 3) * TubePixellation;
  XTRACE(DATA, DEB, "RingOffset %d, TubeOffset %d", RingOffset, TubeOffset);
  return RingOffset + TubeOffset;
}


int BifrostGeometry::yOffset(int TubeId) {
  int Arc = TubeId / 3; // 3 == triplets per arc (for a given ring)
  return Arc * TubesPerTriplet;
}


std::pair<int, float> BifrostGeometry::calcTubeAndPos(
  std::vector<std::pair<double, double>> &Intervals,int AmpA, int AmpB) {

  if (AmpA + AmpB == 0) {
    XTRACE(DATA, DEB, "Sum of amplitudes is 0");
    (*Stats.AmplitudeZero)++;
    return InvalidPos;
  }

  float GlobalPos = 1.0 * AmpA / (AmpA + AmpB); // [0.0 ; 1.0]
  if ((GlobalPos < 0) or (GlobalPos > 1.0)) {
    XTRACE(DATA, WAR, "Pos %f not in unit interval", GlobalPos);
    return InvalidPos;
  }
  int Unit;
  for (Unit = 0; Unit < 3; Unit++) {
    double Min = std::min(Intervals[Unit].first, Intervals[Unit].second);
    double Max = std::max(Intervals[Unit].first, Intervals[Unit].second);
    if ((GlobalPos >= Min) and (GlobalPos <= Max)) {
      break;
    }
  }
  if (Unit == 3) {
    XTRACE(DATA, DEB, "A %d, B %d, GlobalPos %f outside valid region",
           AmpA, AmpB, GlobalPos);
    (*Stats.OutsideTube)++;
    return InvalidPos;
  }

  ///\brief raw unit pos will be in the interval [0;1] regardless of the width
  /// of the interval
  double Lower = Intervals[Unit].first;
  double Upper = Intervals[Unit].second;
  float RawUnitPos = (GlobalPos - Lower)/(Upper - Lower);

  XTRACE(DATA, DEB, "Unit %d, GlobalPos %f, RawUnitPos %f", Unit, GlobalPos, RawUnitPos);
  return std::make_pair(Unit, RawUnitPos);
}


uint32_t BifrostGeometry::calcPixel(DataParser::CaenReadout &Data) {
  int xoff = xOffset(Data.RingId, Data.TubeId);
  int yoff = yOffset(Data.TubeId);

  int Group = Data.RingId * TripletsPerRing + Data.TubeId;
  auto & Intervals = CaenCDCalibration.Intervals[Group];
  std::pair<int, float> TubePos = calcTubeAndPos(Intervals, Data.AmpA, Data.AmpB);

  if (TubePos.first == -1) {
    return 0;
  }

  int ylocal = TubePos.first;
  int xlocal = TubePos.second * (TubePixellation - 1);
  int X = xoff + xlocal;
  int Y = yoff + ylocal;

  uint32_t pixel = ESSGeom->pixel2D(X, Y);
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
