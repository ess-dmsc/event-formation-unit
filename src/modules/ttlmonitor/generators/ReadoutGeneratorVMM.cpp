// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <common/readout/vmm3/VMM3Parser.h>
#include <modules/ttlmonitor/generators/ReadoutGeneratorVMM.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace TTLMonitor {

void ReadoutGeneratorVMM::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  uint32_t TimeLow = TimeLowOffset + TimeToFirstReadout;
  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;

    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    // CSPEC VMM readouts all have DataLength 20
    assert(ReadoutData->DataLength == 20);

    // Monitor is (so far) always on logical ring 11, fen 0
    ReadoutData->RingId = 22;
    ReadoutData->FENId = 0;
    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;
    ReadoutData->OTADC = 0;
    ReadoutData->VMM = 0;
    ReadoutData->Channel = (Readout % 3) & 0x01; // 0 0 1, 0 0 1, ...

    DP += ReadoutDataSize;

    TimeLow += Settings.TicksBtwEvents;
  }
}
} // namespace TTLMonitor
// GCOVR_EXCL_STOP
