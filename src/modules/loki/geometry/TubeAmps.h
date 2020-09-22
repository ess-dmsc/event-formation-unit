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
  void calcPositions(std::uint16_t AmplitudeA, std::uint16_t AmplitudeB,
                    std::uint16_t AmplitudeC, std::uint16_t AmplitudeD) {
    std::uint32_t StrawFrac1 = AmplitudeA + AmplitudeB;
    std::uint32_t StrawFrac2 = AmplitudeC + AmplitudeD;
    std::uint32_t PosFrac1 = AmplitudeA + AmplitudeD;
    std::uint32_t Denominator = StrawFrac1 + StrawFrac2;
    if (Denominator == 0) {
      Stats.AmplitudeZero++;
      StrawId = NStraws;
      PosId = NPos;
      return;
    }
    StrawId = ((NStraws - 1) * StrawFrac1) / Denominator;
    PosId = ((NPos - 1) * PosFrac1) / Denominator;
  }

  void setResolution(uint16_t Resolution) {
    NPos = Resolution;
  }

  struct Stats {
    uint64_t AmplitudeZero{0};
  } Stats;

private:
  const std::uint8_t NStraws{7}; ///< number of straws per tube
  std::uint16_t NPos{512}; ///< resolution of position

public:
  /// holds latest calculated values for straw and position
  /// they will hold out-of-range values if calculation fails
  std::uint8_t StrawId{7};
  std::uint16_t PosId{512};
};

} // nmaespace Loki
