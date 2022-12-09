// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial TTLMon readout
/// based on TTLMon ICD
/// \todo add link
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <common/debug/Trace.h>
#include <math.h>
#include <modules/ttlmonitor/geometry/Parser.h>
#include <modules/ttlmonitor/generators/ReadoutGenerator.h>
#include <time.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace TTLMonitor {

void ReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  uint32_t TimeLow = TimeLowOffset + TimeToFirstReadout;
  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {
    auto ReadoutData = (Parser::Data *)DP;

    ReadoutData->DataLength = sizeof(Parser::Data);
    // TTLMon (new foprmat) readouts all have DataLength 16
    assert(ReadoutData->DataLength == 16);

    // Monitor is (so far) always on logical ring 11, fen 0
    ReadoutData->RingId = 22;
    ReadoutData->FENId = 0;
    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;
    ReadoutData->Pos = 0;
    ReadoutData->ADC = 12345;
    ReadoutData->Channel = (Readout % 3) & 0x01; // 0 0 1, 0 0 1, ...

    DP += sizeof(Parser::Data);

    TimeLow += Settings.TicksBtwEvents;
  }
}
} // namespace TTLMonitor
// GCOVR_EXCL_STOP
