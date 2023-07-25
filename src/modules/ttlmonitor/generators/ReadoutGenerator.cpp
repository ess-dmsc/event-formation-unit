// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
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
#include <modules/ttlmonitor/generators/ReadoutGenerator.h>
#include <modules/ttlmonitor/geometry/Parser.h>

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
    // TTLMon (new format) readouts all have DataLength 16
    assert(ReadoutData->DataLength == 16);

    // Monitor is (so far) always on logical fiber 22 (ring 11), fen 0
    ReadoutData->FiberId = 22;
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
