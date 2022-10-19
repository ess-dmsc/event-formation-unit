/* Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Caen Boron Coated Straw Tubesfunctions
///
/// Ref: Loki TG3.1 Detectors technology "Boron Coated Straw Tubes for LoKI"
/// Davide Raspino 04/09/2019
///
//===----------------------------------------------------------------------===//

#pragma once
#include <cinttypes>
#include <common/debug/Trace.h>
#include <vector>
#include <modules/caen/readout/DataParser.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/Calibration.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_ERR

namespace Caen {

class LokiGeometry {
public:
  /// \brief The four amplitudes measured at certain points in the
  /// Helium tube circuit diagram are used to identify the straw that
  /// detected the neutron and also the position along the straw.
  /// Both of these are calculated at the same time and the result
  /// is stored in the two member variables (StrawId, PosId) if an
  /// invalid input is given the output will be outside the valid
  /// ranges.
  bool calcPositions(std::int16_t AmplitudeA, std::int16_t AmplitudeB,
                     std::int16_t AmplitudeC, std::int16_t AmplitudeD) {
    std::int32_t StrawNum = AmplitudeB + AmplitudeD;
    std::int32_t PosNum = AmplitudeA + AmplitudeB;
    std::int32_t Denominator =
        AmplitudeA + AmplitudeB + AmplitudeC + AmplitudeD;
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

  void setResolution(uint16_t Resolution) { NPos = Resolution; }
  void setCalibration(Calibration Calib) { CaenCalibration = &Calib; }


  struct Stats {
    uint64_t AmplitudeZero{0};
    uint64_t OutsideRegion{0};
  } Stats;

  uint8_t strawCalc(double straw) {
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

  uint32_t calcPixel(PanelGeometry &Panel, uint8_t FEN,
                      DataParser::CaenReadout &Data){
    uint8_t TubeGroup = FEN;
    uint8_t LocalTube = Data.TubeId;

    bool valid =
        calcPositions(Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);


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

    if ((GlobalStraw < MinStraw) or
      (GlobalStraw > MaxStraw)) {
      Stats.OutsideRegion++;
      return 0;
    }

    uint16_t CalibratedPos =
      CaenCalibration->strawCorrection(GlobalStraw, PosVal);
    XTRACE(EVENT, DEB, "calibrated pos: %u", CalibratedPos);

    uint32_t PixelId =
      ESSGeom.pixel2D(CalibratedPos, GlobalStraw);

    XTRACE(EVENT, DEB, "xpos %u (calibrated: %u), ypos %u, pixel: %u", PosVal,
         CalibratedPos, GlobalStraw, PixelId);

    return PixelId;
}

private:
  const std::uint8_t NStraws{7}; ///< number of straws per tube
  std::uint16_t NPos{512};       ///< resolution of position
  uint16_t MinStraw{0};          /// ported over from the original LoKI module
  uint16_t MaxStraw{65535};      /// with a todo note to remove
  Calibration *CaenCalibration;
  ESSGeometry ESSGeom;

public:
  /// holds latest calculated values for straw and position
  /// they will hold out-of-range values if calculation fails
  std::uint8_t StrawId{7};
  double PosVal{512.0};
};

} // namespace Caen
