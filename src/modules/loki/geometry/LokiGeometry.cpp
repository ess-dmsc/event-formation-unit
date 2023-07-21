// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for Loki:
/// ...
///
//===----------------------------------------------------------------------===//

#include <modules/loki/geometry/LokiGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

LokiGeometry::LokiGeometry(Config &CaenConfiguration)
    : Panels(CaenConfiguration.Panels) {
  ESSGeom = new ESSGeometry(
      CaenConfiguration.Resolution,
      CaenConfiguration.NTubesTotal * PanelGeometry::NStraws, 1, 1);
  setResolution(CaenConfiguration.Resolution);
  MaxRing = CaenConfiguration.MaxRing;
}

uint32_t LokiGeometry::calcPixel(DataParser::CaenReadout &Data) {
  uint8_t TubeGroup = Data.FENId;

  bool valid = calcPositions(Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);
  int Ring = Data.FiberId/2;
  PanelGeometry Panel = Panels[Ring];
  XTRACE(DATA, DEB, "Fiber ID %u, RingId %d", Data.FiberId, Ring);
  if (not valid) {
    return 0;
  }
  XTRACE(DATA, DEB, "Valid pixel id calculated");
  /// Globalstraw is per its definition == Y
  uint32_t GlobalStraw = Panel.getGlobalStrawId(TubeGroup, Data.Group, StrawId);

  XTRACE(EVENT, DEB, "global straw: %u", GlobalStraw);
  if (GlobalStraw == Panel.StrawError) {
    XTRACE(EVENT, WAR, "Invalid straw id: %d", GlobalStraw);
    return 0;
  }

  int Group = GlobalStraw / 7;
  int Unit = StrawId;
  double CalibratedUnitPos =
      CaenCDCalibration.posCorrection(Group, Unit, PosVal);
  uint16_t CalibratedPos = CalibratedUnitPos * (NPos - 1);
  XTRACE(EVENT, DEB, "Group %d, Unit %d - calibrated unit pos: %g, pos %d",
         Group, Unit, CalibratedUnitPos, CalibratedPos);

  uint32_t PixelId = ESSGeom->pixel2D(CalibratedPos, GlobalStraw);

  XTRACE(EVENT, DEB, "xpos %u (calibrated: %u), ypos %u, pixel: %u", PosVal,
         CalibratedPos, GlobalStraw,
         PixelId); ///\todo this print statement prints a random number for
                   /// pixel id
  XTRACE(EVENT, DEB, "Pixel is %u", PixelId);
  return PixelId;
}

bool LokiGeometry::validateData(DataParser::CaenReadout &Data) {
  unsigned int Ring = Data.FiberId / 2;
  if (Ring >= Panels.size()) {
    XTRACE(DATA, WAR, "RINGId %u is incompatible with #panels: %u", Ring,
           Panels.size());
    Stats.RingErrors++;
    return false;
  }
  XTRACE(DATA, DEB, "Panels size %u", Panels.size());

  auto Panel = Panels[Ring];

  if (Data.FENId >= Panel.getMaxGroup()) {
    XTRACE(DATA, WAR, "FENId %u outside valid range 0 - %u", Data.FENId,
           Panel.getMaxGroup() - 1);
    Stats.FENErrors++;
    return false;
  }
  XTRACE(DATA, DEB, "FENId %d, Max FENId %d", Data.FENId,
         Panel.getMaxGroup() - 1);
  return true;
}

bool LokiGeometry::calcPositions(std::int16_t AmplitudeA,
                                 std::int16_t AmplitudeB,
                                 std::int16_t AmplitudeC,
                                 std::int16_t AmplitudeD) {
  std::int32_t StrawNum = AmplitudeB + AmplitudeD;
  std::int32_t PosNum = AmplitudeA + AmplitudeB;
  std::int32_t Denominator = AmplitudeA + AmplitudeB + AmplitudeC + AmplitudeD;
  XTRACE(INIT, DEB, "StrawNum: %d, PosNum: %d, Denominator: %d", StrawNum,
         PosNum, Denominator);
  if (Denominator == 0) {
    XTRACE(INIT, WAR,
           "Denominator is 0, StrawNum: %d, PosNum: %d, "
           " Denominator: %d,  A %d, B %d, C %d, D %d",
           StrawNum, PosNum, Denominator, AmplitudeA, AmplitudeB, AmplitudeC,
           AmplitudeD);
    Stats.AmplitudeZero++;
    StrawId = NStraws;
    PosVal = NPos;
    return false;
  }
  double dStrawId = ((NStraws - 1) * StrawNum * 1.0) / Denominator;
  StrawId = strawCalc(dStrawId);
  PosVal = (PosNum * 1.0) / Denominator;
  XTRACE(INIT, DEB, "dStraw %f, StrawId %d, PosNum: %d, PosVal: %f", dStrawId,
         StrawId, PosNum, PosVal);
  return true;
}

uint8_t LokiGeometry::strawCalc(double straw) {
  // limits is a vector defined in LokiGeometry.h
  if (straw <= limits[0])
    return 0;
  else if (straw <= limits[1])
    return 1;
  else if (straw <= limits[2])
    return 2;
  else if (straw <= limits[3])
    return 3;
  else if (straw <= limits[4])
    return 4;
  else if (straw <= limits[5])
    return 5;
  else
    return 6;
}

} // namespace Caen
