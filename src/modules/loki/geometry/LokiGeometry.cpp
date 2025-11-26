// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for Loki:
///
/// \todo Note there is some confusion about the LOKI amplitudes.
/// Amplitudes A and B are actually swapped (in firmware) and so are
/// amplitudes C and D.
/// This should be fixed at some point.
//===----------------------------------------------------------------------===//

#include <modules/loki/geometry/LokiGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

LokiGeometry::LokiGeometry(Statistics &Stats, Config &CaenConfiguration)
    : Geometry(Stats, CaenConfiguration.CaenParms.MaxRing,
               CaenConfiguration.CaenParms.MaxFEN,
               CaenConfiguration.LokiConf.Parms.TotalGroups),
      ESSGeometry(CaenConfiguration.CaenParms.Resolution,
                  CaenConfiguration.LokiConf.Parms.TotalGroups * UNITS_PER_STRAW, 1, 1),
      Conf(CaenConfiguration),
      StrawResolution(CaenConfiguration.CaenParms.Resolution) {}

std::pair<int, double> LokiGeometry::calcUnitAndPos(int GlobalGroup, int AmpA,
                                                    int AmpB, int AmpC,
                                                    int AmpD) const {

  XTRACE(DATA, DEB, "calcUnitAndPos: GlobalGroup %d", GlobalGroup);

  // While formally wrong, this calculation is not affected by the
  // amplitude swapping.
  int Denominator = AmpA + AmpB + AmpC + AmpD;

  if (Denominator == 0) {
    XTRACE(DATA, DEB, "Sum of amplitudes is 0");
    CaenStats.AmplitudeZero++;
    return InvalidPos;
  }

  // Calculate uncorrected position as float
  // While formally wrong, this calculation is not affected by the
  // amplitude swapping.
  double UncorrPos = 1.0 * (AmpA + AmpB) / Denominator; // [0.0 ; 1.0]
  if ((UncorrPos < 0) or (UncorrPos > 1.0)) {
    XTRACE(DATA, WAR, "UncorrPos %f not in unit interval", UncorrPos);
    return InvalidPos;
  }

  // Calculate straw as float in the unit interval [0.0 ; 1.0]
  /// \todo This implementation compensates for the amplitude swap.
  /// and has been validated by Wireshark capture and the LOKI pattern
  /// generator functionality.
  double Straw = 1.0 * (AmpB + AmpD) / Denominator;
  int Unit = CaenCDCalibration.getUnitId(GlobalGroup, Straw);
  if (Unit == -1) {
    XTRACE(DATA, DEB, "B %d, D %d, Straw %f outside specified region", AmpB,
           AmpD, Straw);
    return InvalidPos;
  }

  return std::make_pair(Unit, UncorrPos);
}

uint32_t LokiGeometry::calcPixelImpl(const DataParser::CaenReadout &Data) const {
  int Ring = Data.FiberId / 2;
  int FEN = Data.FENId;
  int Group = Data.Group; // local group for a FEN

  XTRACE(DATA, DEB, "Fiber ID %u, Ring %d", Data.FiberId, Ring);

  uint32_t GlobalGroup = Conf.LokiConf.getGlobalGroup(Ring, FEN, Group);
  XTRACE(DATA, DEB, "FEN %d, LocalGroup %d, GlobalGroup %d", FEN, Group,
         GlobalGroup);

  std::pair<int, double> UnitPos =
      calcUnitAndPos(GlobalGroup, Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);
  XTRACE(DATA, DEB, "Unit %d, GlobalPos %f", UnitPos.first, UnitPos.second);

  if (UnitPos.first == -1) {
    return 0;
  }

  uint32_t GlobalUnit =
      Conf.LokiConf.getY(Ring, FEN, Data.Group, UnitPos.first);

  double CalibratedUnitPos = CaenCDCalibration.posCorrection(
      GlobalGroup, UnitPos.first, UnitPos.second);
  uint16_t CalibratedPos = round(CalibratedUnitPos * (StrawResolution - 1));
  XTRACE(EVENT, DEB, "Group %d, Unit %d - calibrated unit pos: %g, pos %d",
         GlobalGroup, UnitPos.first, CalibratedUnitPos, CalibratedPos);

  uint32_t PixelId = pixel2D(CalibratedPos, GlobalUnit);

  ///\todo this print statement prints a random number for pixel id
  XTRACE(EVENT, DEB, "xpos %f (calibrated: %u), ypos %u, pixel: %u",
         UnitPos.second, CalibratedPos, GlobalUnit, PixelId);

  XTRACE(EVENT, DEB, "Pixel is %u", PixelId);
  return PixelId;
}

bool LokiGeometry::validateReadoutData(const DataParser::CaenReadout &Data) const {
  auto Ring = Data.FiberId / 2;
  auto &Cfg = Conf.LokiConf.Parms;

  return validateAll(
      [&]() {
        if (Ring >= Cfg.NumRings) {
          XTRACE(DATA, WAR, "RINGId %u is >= %u", Ring, Cfg.NumRings);
          BaseCounters.RingErrors++;
          return false;
        } else {
          return true;
        }
      },
      [&]() {
        auto Bank = Cfg.Rings[Ring].Bank;
        if (Bank == -1) {
          XTRACE(DATA, WAR, "RINGId %u is uninitialised", Ring);
          BaseCounters.RingMappingErrors++;
          return false;
        } else {
          return true;
        }
      },
      [&]() {
        auto FENs = Cfg.Rings[Ring].FENs;
        if (Data.FENId >= FENs) {
          XTRACE(DATA, DEB, "FENId %d, Max FENId %d", Data.FENId, FENs - 1);
          XTRACE(DATA, WAR, "FENId %u outside valid range 0 - %u", Data.FENId,
                 FENs);
          BaseCounters.TopologyError++;
          return false;
        } else {
          return true;
        }
      });
}

} // namespace Caen
