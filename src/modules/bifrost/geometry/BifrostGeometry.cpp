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
  int Ring = Data.FiberId / 2;
  XTRACE(DATA, DEB, "Fiber %u, Ring %d, FEN %u, Tube %u", Data.FiberId, Ring,
        Data.FENId, Data.TubeId);

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

  if (Data.TubeId > MaxTube) {
    XTRACE(DATA, WAR, "Tube %d is incompatible with config", Data.TubeId);
    Stats.GroupErrors++;
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

std::pair<int, double> BifrostGeometry::calcUnitAndPos(int Group, int AmpA,
                                                       int AmpB) {

  if (AmpA + AmpB == 0) {
    XTRACE(DATA, DEB, "Sum of amplitudes is 0");
    Stats.AmplitudeZero++;
    return InvalidPos;
  }

  double GlobalPos = 1.0 * AmpA / (AmpA + AmpB); // [0.0 ; 1.0]
  if ((GlobalPos < 0) or (GlobalPos > 1.0)) {
    XTRACE(DATA, WAR, "Pos %f not in unit interval", GlobalPos);
    return InvalidPos;
  }

  int Unit = CaenCDCalibration.getUnitId(Group, GlobalPos);
  if (Unit == -1) {
    XTRACE(DATA, DEB, "A %d, B %d, GlobalPos %f outside valid region", AmpA,
           AmpB, GlobalPos);
    return InvalidPos;
  }

  ///\brief raw unit pos will be in the interval [0;1] regardless of the width
  /// of the interval
  auto &Intervals = CaenCDCalibration.Intervals[Group];
  double Lower = Intervals[Unit].first;
  double Upper = Intervals[Unit].second;
  double RawUnitPos = (GlobalPos - Lower) / (Upper - Lower);

  XTRACE(DATA, DEB, "Unit %d, GlobalPos %f, RawUnitPos %f", Unit, GlobalPos,
         RawUnitPos);
  return std::make_pair(Unit, RawUnitPos);
}

uint32_t BifrostGeometry::calcPixel(DataParser::CaenReadout &Data) {
  int Ring = Data.FiberId / 2;
  int xoff = xOffset(Ring, Data.TubeId);
  int yoff = yOffset(Data.TubeId);

  int Group = Ring * TripletsPerRing + Data.TubeId;
  std::pair<int, double> UnitPos = calcUnitAndPos(Group, Data.AmpA, Data.AmpB);

  if (UnitPos.first == -1) {
    return 0;
  }

  int ylocal = UnitPos.first;
  int xlocal = UnitPos.second * (TubePixellation - 1);
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
