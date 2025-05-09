// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
// based on TREX ICD document
// https://project.esss.dk/owncloud/index.php/f/14482406
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <common/debug/Trace.h>
#include <math.h>
#include <modules/trex/generators/LETReadoutGenerator.h>
#include <time.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Trex {

void LETReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  uint16_t XGlobal = 0;
  uint16_t XLocal = 0;
  uint16_t YLocal = 0;
  uint8_t VMM = 0;
  uint16_t Channel = 0;

  for (uint32_t Readout = 0; Readout < NumberOfReadouts; Readout++) {

    XTRACE(DATA, DEB, "TimeLow = %u, TimeHigh = %u", getReadoutTimeLow(),
           getReadoutTimeHigh());
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;

    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    // TREX VMM readouts all have DataLength 20
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = getReadoutTimeHigh();
    ReadoutData->TimeLow = getReadoutTimeLow();
    ReadoutData->OTADC = 1000;

    // TREX is 16 wires deep in Z direction
    // X is selected as a number between 0 and 11 as there are
    // 2 columns of 6 wires in the LET setup
    if ((GlobalReadout % 16) == 0) {
      XGlobal = Fuzzer.random8() * 12 / 255;
    }
    // Forms a tick shape, and stretches it into a taller rectangle
    // as for LET MaxX = 11 and MaxY = 50
    YLocal = 4 * abs(XGlobal - 2);

    // Readout generated for LET test, with Fiber 0(Ring 0)
    ReadoutData->FiberId = 0;

    // Each column is 6 wires wide
    // Select the FEN based on whether XGlobal is in column 0 or column 1
    // Initialise XLocal as the local X value within each column
    if (XGlobal < 6) {
      ReadoutData->FENId = 0;
      XLocal = XGlobal;
    } else {
      ReadoutData->FENId = 1;
      XLocal = XGlobal - 6;
    }

    // Wire X and Z direction
    // All channel calculations are based on ICD linked at top of file
    if ((GlobalReadout % 2) == 0) {
      uint8_t ZLocal = 12 - abs(XGlobal - 2);
      if (XLocal < 2) {
        VMM = 0;
        Channel = (XLocal * 16) + 32 + ZLocal;
      } else {
        VMM = 1;
        Channel = (XLocal - 2) * 16 + ZLocal;
      }
      XTRACE(DATA, DEB, "Readout for Wires");
    }
    // Grid Y direction
    // Mappings of Y coordinates to channels and VMMs is complicated
    // Details are in ICD, with useful figure
    else {
      VMM = 2;
      Channel = 50 - YLocal;
      XTRACE(DATA, DEB, "Readout for grids");
    }

    ReadoutData->VMM = VMM;
    ReadoutData->Channel = Channel;

    DP += ReadoutDataSize;
    XTRACE(DATA, DEB, "Coordinate XGlobal %u, XLocal %u, YLocal %u", XGlobal,
           XLocal, YLocal);

    GlobalReadout++;
    XTRACE(DATA, DEB, "TimeLow = %u, TimeHigh - %u", getReadoutTimeLow(),
           getReadoutTimeHigh());
  }
}

} // namespace Trex

// GCOVR_EXCL_STOP
