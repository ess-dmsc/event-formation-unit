// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for Tbl3He:
/// https://project.esss.dk/nextcloud/index.php/s/3SM88TjNfKxyPrz
///
//===----------------------------------------------------------------------===//

#include <modules/tbl3he/geometry/Tbl3HeGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

Tbl3HeGeometry::Tbl3HeGeometry(Config &CaenConfiguration)
    : Conf(CaenConfiguration.Tbl3HeConf) {
  ESSGeom = new ESSGeometry(100, 8, 1, 1);
  setResolution(Conf.Parms.Resolution);
}


///\todo refactoring oportunity: this code is nearly identical to the code in bifrost
std::pair<int, double> Tbl3HeGeometry::calcUnitAndPos(int Group, int AmpA,
                                                       int AmpB) {
  int MinAmpl = Conf.Parms.MinValidAmplitude;
  if ((AmpA < MinAmpl) or (AmpB < MinAmpl)) { ///\todo replace with configuration
    XTRACE(DATA, DEB, "At least one amplitude is too low");
    Stats.AmplitudeLow++;
    return InvalidPos;
  }

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

bool Tbl3HeGeometry::validateData(DataParser::CaenReadout &Data) {
  int Ring = Data.FiberId / 2;
  XTRACE(DATA, DEB, "Fiber %u, Ring %d, FEN %u, Group %u", Data.FiberId, Ring,
         Data.FENId, Data.Group);


  if (Ring > Conf.Parms.MaxRing) {
    XTRACE(DATA, WAR, "RING %d is incompatible with config (MaxRing %d)", Ring, MaxRing);
    Stats.RingErrors++;
    return false;
  }


  if (not Conf.TopologyMapPtr->isValue(Ring, Data.FENId)) {
    XTRACE(DATA, WAR, "Ring %d, FEN %d is incompatible with config", Ring, Data.FENId);
    Stats.TopologyErrors++;
    return false;
  }

  if (Data.Group > Conf.Parms.MaxGroup) {
    XTRACE(DATA, WAR, "Group %d is incompatible with config", Data.Group);
    Stats.GroupErrors++;
    return false;
  }
  return true;
}


/// \brief calculate the pixel id from the readout data
/// \return 0 for invalid pixel, nonzero for good pixels
uint32_t Tbl3HeGeometry::calcPixel(DataParser::CaenReadout &Data) {
  int Ring = Data.FiberId / 2;
  int Tube = Data.Group;

  int Bank = Conf.TopologyMapPtr->get(Ring, Data.FENId)->Bank;

  XTRACE(DATA, DEB, "FiberId %d, Ring %d, Group %d, Bank %d", Data.FiberId,
         Ring, Tube, Bank);

  // Get global Group id - will be used for calibration
  int GlobalGroup = Bank * 4 + Tube;

  std::pair<int, double> UnitPos = calcUnitAndPos(GlobalGroup, Data.AmpA, Data.AmpB);

  if (UnitPos.first == -1) {
    return 0;
  }

  double CalibratedUnitPos =
      CaenCDCalibration.posCorrection(GlobalGroup, UnitPos.first, UnitPos.second);

  int xlocal = CalibratedUnitPos * (UnitPixellation - 1);

  int X = xlocal;
  int Y = GlobalGroup;

  uint32_t pixel = ESSGeom->pixel2D(X, Y);
  if (pixel == 0) {
    XTRACE(DATA, WAR, "xlocal %d, X %d, Y %d, pixel %hu",
           xlocal, X, Y, pixel);
  } else {
    XTRACE(DATA, DEB, "xlocal %d, X %d, Y %d, pixel %hu",
           xlocal, X, Y, pixel);
  }

  return pixel;
}

size_t Tbl3HeGeometry::numSerializers() const {
  return 2;
}

size_t Tbl3HeGeometry::calcSerializer(DataParser::CaenReadout &Data) const {
  int Ring = Data.FiberId / 2;
  return Conf.TopologyMapPtr->get(Ring, Data.FENId)->Bank;
}

std::string Tbl3HeGeometry::serializerName(size_t Index) const {
  return fmt::format("bank{}", Index);
}

} // namespace Caen
