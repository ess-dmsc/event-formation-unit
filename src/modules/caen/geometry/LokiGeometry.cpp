// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from digital identifiers, see latest reviewed
/// ICD for Loki:
/// ...
///
//===----------------------------------------------------------------------===//

#include <modules/caen/geometry/LokiGeometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

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
  PosVal = ((NPos - 1) * PosNum * 1.0) / Denominator;
  XTRACE(INIT, DEB, "dStraw %f, StrawId %d, PosNum: %d, PosVal: %f", dStrawId,
         StrawId, PosNum, PosVal);
  return true;
}

uint8_t LokiGeometry::strawCalc(double straw) {
  std::vector<double> limits = {0.7, 1.56, 2.52, 3.54, 4.44, 5.3};
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

uint32_t LokiGeometry::calcPixel(PanelGeometry &Panel, uint8_t FEN,
                                 DataParser::CaenReadout &Data) {
  uint8_t TubeGroup = FEN;
  uint8_t LocalTube = Data.TubeId;

  bool valid = calcPositions(Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);

  if (not valid) {
    return 0;
  }
  /// Globalstraw is per its definition == Y
  uint32_t GlobalStraw = Panel.getGlobalStrawId(TubeGroup, LocalTube, StrawId);

  XTRACE(EVENT, DEB, "global straw: %u", GlobalStraw);
  if (GlobalStraw == Panel.StrawError) {
    XTRACE(EVENT, WAR, "Invalid straw id: %d", GlobalStraw);
    return 0;
  }

  uint16_t CalibratedPos = CaenCalibration.strawCorrection(GlobalStraw, PosVal);
  XTRACE(EVENT, DEB, "calibrated pos: %u", CalibratedPos);

  uint32_t PixelId = ESSGeom->pixel2D(CalibratedPos, GlobalStraw);
  // ESSGeom.pixel2D(PosVal, GlobalStraw);

  XTRACE(EVENT, DEB, "xpos %u (calibrated: %u), ypos %u, pixel: %u", PosVal,
         CalibratedPos, GlobalStraw,
         PixelId); ///\todo this print statement prints a random number for
                   ///pixel id
  XTRACE(EVENT, DEB, "Pixel is %u", PixelId);
  return PixelId;
}

} // namespace Caen
