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
      CaenConfiguration.NGroupsTotal * PanelGeometry::NUnits, 1, 1);
  setResolution(CaenConfiguration.Resolution);
  MaxRing = CaenConfiguration.MaxRing;
}

uint32_t LokiGeometry::calcPixel(DataParser::CaenReadout &Data) {
  uint8_t GroupBank = Data.FENId; // one FEN has 8 Groups

  bool valid = calcPositions(Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);
  int Ring = Data.FiberId/2;
  PanelGeometry Panel = Panels[Ring];
  XTRACE(DATA, DEB, "Fiber ID %u, RingId %d", Data.FiberId, Ring);
  if (not valid) {
    return 0;
  }
  XTRACE(DATA, DEB, "Valid pixel id calculated");
  /// GlobalUnit is per its definition == Y
  uint32_t GlobalUnit = Panel.getGlobalUnitId(GroupBank, Data.Group, UnitId);

  XTRACE(EVENT, DEB, "global straw: %u", GlobalUnit);
  if (GlobalUnit == Panel.UnitError) {
    XTRACE(EVENT, WAR, "Invalid straw id: %d", GlobalUnit);
    return 0;
  }

  int Group = GlobalUnit / 7;
  int Unit = UnitId;
  double CalibratedUnitPos =
      CaenCDCalibration.posCorrection(Group, Unit, PosVal);
  uint16_t CalibratedPos = CalibratedUnitPos * (NPos - 1);
  XTRACE(EVENT, DEB, "Group %d, Unit %d - calibrated unit pos: %g, pos %d",
         Group, Unit, CalibratedUnitPos, CalibratedPos);

  uint32_t PixelId = ESSGeom->pixel2D(CalibratedPos, GlobalUnit);

  XTRACE(EVENT, DEB, "xpos %u (calibrated: %u), ypos %u, pixel: %u", PosVal,
         CalibratedPos, GlobalUnit,
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
  std::int32_t UnitNum = AmplitudeB + AmplitudeD;
  std::int32_t PosNum = AmplitudeA + AmplitudeB;
  std::int32_t Denominator = AmplitudeA + AmplitudeB + AmplitudeC + AmplitudeD;
  XTRACE(INIT, DEB, "UnitNum: %d, PosNum: %d, Denominator: %d", UnitNum,
         PosNum, Denominator);
  if (Denominator == 0) {
    XTRACE(INIT, WAR,
           "Denominator is 0, UnitNum: %d, PosNum: %d, "
           " Denominator: %d,  A %d, B %d, C %d, D %d",
           UnitNum, PosNum, Denominator, AmplitudeA, AmplitudeB, AmplitudeC,
           AmplitudeD);
    Stats.AmplitudeZero++;
    UnitId = NUnits;
    PosVal = NPos;
    return false;
  }
  double dUnitId = ((NUnits - 1) * UnitNum * 1.0) / Denominator;
  UnitId = getUnitId(dUnitId);
  PosVal = (PosNum * 1.0) / Denominator;
  XTRACE(INIT, DEB, "dUnit %f, UnitId %d, PosNum: %d, PosVal: %f", dUnitId,
         UnitId, PosNum, PosVal);
  return true;
}

// convert from Unit value [0.0; 1.0] to integer UnitId
// \todo replace with commom caen code
uint8_t LokiGeometry::getUnitId(double value) {
  // limits is a vector defined in LokiGeometry.h
  if (value <= limits[0])
    return 0;
  else if (value <= limits[1])
    return 1;
  else if (value <= limits[2])
    return 2;
  else if (value <= limits[3])
    return 3;
  else if (value <= limits[4])
    return 4;
  else if (value <= limits[5])
    return 5;
  else
    return 6;
}

} // namespace Caen
