// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
/// for Loki ICD:
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <common/debug/Trace.h>
#include <math.h>
#include <modules/loki/generators/ReadoutGenerator.h>
#include <time.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {

void ReadoutGenerator::generateData() {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;
  uint8_t LokiDataSize = sizeof(DataParser::LokiReadout);

  for (uint32_t Readout = 0; Readout < Settings.NumReadouts; Readout++) {

    auto ReadoutData = (DataParser::LokiReadout *)DP;

    ReadoutData->DataLength = ReadoutDataSize;
    assert(ReadoutData->DataLength == LokiDataSize);

    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;

    ReadoutData->RingId = (Readout/10) % Settings.NRings;
    ReadoutData->FENId = Readout % 8;
    ReadoutData->DataLength = LokiDataSize;

    ReadoutData->TubeId = (Readout/10) % 8;
    ReadoutData->AmpA = Readout;
    ReadoutData->AmpB = 1;
    ReadoutData->AmpC = 1;
    ReadoutData->AmpD = 100;
    // printf("Readout %d: Ring %u, FEN %u, Tube %u, A %u\n", Readout,
    //        ReadoutData->RingId,
    //        ReadoutData->FENId, ReadoutData->TubeId, ReadoutData->AmpA);
    DP += LokiDataSize;
    ///

    // All readouts are events for loki
    TimeLow += Settings.TicksBtwEvents;

    if (TimeLow >= 88052499){
      TimeLow -= 88052499;
      TimeHigh += 1;
    }
  }
}
} // namespace Loki
// GCOVR_EXCL_STOP
