/* Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Loki Boron Coated Straw Tubesfunctions
///
/// Ref: Loki TG3.1 Detectors technology "Boron Coated Straw Tubes for LoKI"
/// Davide Raspino 04/09/2019
///
//===----------------------------------------------------------------------===//

#pragma once
#include <cinttypes>
#include <common/Trace.h>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_ERR

namespace Loki {

class TubeAmps {
public:
  /// \brief The four amplitudes measured at certain points in the
  /// Helium tube circuit diagram are used to identify the straw that
  /// detected the neutron and also the position along the straw.
  /// Both of these are calculated at the same time and the result
  /// is stored in the two member variables (StrawId, PosId) if an
  /// invalid input is given the output will be outside the valid
  /// ranges.
  bool calcPositions(std::uint16_t AmplitudeA, std::uint16_t AmplitudeB,
                     std::uint16_t AmplitudeC, std::uint16_t AmplitudeD) {
    std::uint32_t StrawNum = AmplitudeA + AmplitudeC;
    std::uint32_t PosNum = AmplitudeA + AmplitudeB;
    std::uint32_t Denominator =
        AmplitudeA + AmplitudeB + AmplitudeC + AmplitudeD;
    XTRACE(INIT, DEB, "StrawNum: %u, PosNum: %u, Denominator: %u", StrawNum,
           PosNum, Denominator);
    if (Denominator == 0) {
      XTRACE(INIT, WAR, "StrawNum: %u, PosNum: %u, Denominator: %u", StrawNum,
             PosNum, Denominator);
      Stats.AmplitudeZero++;
      StrawId = NStraws;
      PosVal = NPos;
      return false;
    }
    double dStrawId = ((NStraws - 1) * StrawNum * 1.0) / Denominator;
    StrawId = strawCalc(dStrawId);
    PosVal = ((NPos - 1) * PosNum * 1.0) / Denominator;
    XTRACE(INIT, DEB, "dStraw %f, StrawId %u, PosId: %u", dStrawId, StrawId);
    return true;
  }

  void setResolution(uint16_t Resolution) { NPos = Resolution; }

  struct Stats {
    uint64_t AmplitudeZero{0};
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

private:
  const std::uint8_t NStraws{7}; ///< number of straws per tube
  std::uint16_t NPos{512};       ///< resolution of position

public:
  /// holds latest calculated values for straw and position
  /// they will hold out-of-range values if calculation fails
  std::uint8_t StrawId{7};
  double PosVal{512.0};
};

} // namespace Loki
