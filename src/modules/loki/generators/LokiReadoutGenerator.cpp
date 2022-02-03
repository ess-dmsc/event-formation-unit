// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
/// based on LoKI ICD:
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <common/debug/Trace.h>
#include <math.h>
#include <modules/loki/generators/LokiReadoutGenerator.h>
#include <time.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

void Loki::LokiReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {

    auto ReadoutData = (DataParser::LokiReadout *)DP;

    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    /// \todo LOKI readouts all have DataLength xx
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;

    /// \todo ... add code here for ring, fen,amplitudes

    ///

    // All readouts are events for loki
    TimeLow += Settings.TicksBtwEvents;

    if (TimeLow >= 88052499){
      TimeLow -= 88052499;
      TimeHigh += 1;
    }
  }
}

// GCOVR_EXCL_STOP
