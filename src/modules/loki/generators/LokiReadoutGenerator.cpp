// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial CAEN readouts
/// for Loki ICD:
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

namespace Caen {

void LokiReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;
  uint8_t LokiDataSize = sizeof(DataParser::CaenReadout);

  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {

    auto ReadoutData = (DataParser::CaenReadout *)DP;

    ReadoutData->DataLength = ReadoutDataSize;
    assert(ReadoutData->DataLength == LokiDataSize);

    ReadoutData->TimeHigh = getReadoutTimeHigh();
    ReadoutData->TimeLow = getReadoutTimeLow();

    ReadoutData->FiberId = (Readout / 10) % Settings.NFibers;
    ReadoutData->FENId = Readout % 8;
    ReadoutData->DataLength = LokiDataSize;

    ReadoutData->Group = (Readout / 10) % 8;
    ReadoutData->AmpA = Readout;
    ReadoutData->AmpB = 1;
    ReadoutData->AmpC = 1;
    ReadoutData->AmpD = 100;
    // printf("Readout %d: Fiber %u, FEN %u, Group %u, A %u\n", Readout,
    //        ReadoutData->FiberId,
    //        ReadoutData->FENId, ReadoutData->Group, ReadoutData->AmpA);
    DP += LokiDataSize;

    // increment redout time for next redout
    nextReadoutTime();
  }
}

ESSReadout::ESSTime::PulseTime LokiReadoutGenerator::generatePulseTime() {
  return pulseTime = ESSReadout::ESSTime::PulseTime(time(NULL));
}

} // namespace Caen
// GCOVR_EXCL_STOP
