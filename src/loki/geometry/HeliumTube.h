/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Loki Helium tube functions
///
/// Ref: Loki TG3.1 Detectors technology "Boron Coated Straw Tubes for LoKI"
/// Davide Raspino 04/09/2019
///
//===----------------------------------------------------------------------===//



#pragma once

namespace Loki {

class HeliumTube {
public:

  void calcPositions(uint16_t ampA, uint16_t ampB, uint16_t ampC, uint16_t ampD) {
    uint32_t StrawFrac1 = ampA + ampB;
    uint32_t StrawFrac2 = ampC + ampD;
    uint32_t PosFrac1 = ampA + ampD;
    uint32_t Denominator = StrawFrac1 + StrawFrac2;
    if (Denominator == 0) {
      Stats.AmplitudeZero++;
      StrawId = NStraws;
      PosId = NPos;
      return;
    }
    StrawId = ((NStraws - 1) * StrawFrac1) / Denominator;
    PosId = ((NPos - 1) * PosFrac1) / Denominator;
  }

  struct Stats {
    uint64_t AmplitudeZero{0};
  } Stats;


private:
  const uint8_t NStraws{7};
  const uint16_t NPos{512};

public:
  /// holds latest calculated values for straw and position
  /// they will hold out of range values if calculation fails
  uint8_t StrawId{NStraws};
  uint16_t PosId{NPos};
};

} // nmaespace Loki
