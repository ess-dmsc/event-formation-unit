// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Timepix3 Modules
//===----------------------------------------------------------------------===//

#pragma once

#include "readout/PixelEventHandler.h"
#include "readout/TimingEventHandler.h"
#include <cstdint>

namespace Timepix3 {

// | 8b  |      12b        |     35b   |  4b   |  5b  |
// | 0x6 | Trigger counter | Timestamp | Stamp | Resv |

// clang-format off
#define TYPE_MASK 0xF000000000000000
#define TYPE_OFFS 60

// pixel type data
#define PIXEL_DCOL_MASK     0x0FE0000000000000
#define PIXEL_SPIX_MASK     0x001F800000000000
#define PIXEL_PIX_MASK      0x0000700000000000
#define PIXEL_TOA_MASK      0x00000FFFC0000000
#define PIXEL_TOT_MASK      0x000000003FF00000
#define PIXEL_FTOA_MASK     0x00000000000F0000
#define PIXEL_SPTIME_MASK   0x000000000000FFFF
#define PIXEL_DCOL_OFFSET   52
#define PIXEL_SPIX_OFFSET   45
#define PIXEL_PIX_OFFSET    44
#define PIXEL_TOA_OFFSET    28
#define PIXEL_TOT_OFFSET    20
#define PIXEL_FTOA_OFFSET   16

// TDC type data
#define TDC_TYPE_MASK             0x0F00000000000000
#define TDC_TRIGGERCOUNTER_MASK   0x00FFF00000000000
#define TDC_TIMESTAMP_MASK        0x00000FFFFFFFFE00
#define TDC_STAMP_MASK            0x00000000000001E0
#define TDC_TYPE_OFFSET           56
#define TDC_TRIGGERCOUNTER_OFFSET 44
#define TDC_TIMESTAMP_OFFSET      9
#define TDC_STAMP_OFFSET          5

// Global Timestamp type data
#define GLOBAL_TIMESTAMP_MASK     0x00FFFFFFFFFFFF00
#define GLOBAL_STAMP_MASK         0x00000000000000F0
#define GLOBAL_TIMESTAMP_OFFSET   8
#define GLOBAL_STAMP_OFFSET       4

#define EVR_READOUT_TYPE          1

#define EVR_READOUT_TYPE          1
// clang-format on

struct EVRReadout {
  const uint8_t Type;
  const uint8_t Unused;
  const uint16_t Unused2;
  const uint32_t Counter;
  const uint32_t PulseTimeSeconds;
  const uint32_t PulseTimeNanoSeconds;
  const uint32_t PrevPulseTimeSeconds;
  const uint32_t PrevPulseTimeNanoSeconds;
} __attribute__((__packed__));

struct Timepix3TDCReadout {
  uint8_t type;
  uint16_t trigger_counter;
  uint64_t timestamp;
  uint8_t stamp;
} __attribute__((__packed__));

struct Timepix3GlobalTimeReadout {
  uint64_t Timestamp;
  uint8_t Stamp;
}; // as above, the readouts aren't packed this way

class DataParser {
public:
  const unsigned int MaxReadoutsInPacket{500};

  DataParser(Counters &counters,
             Observer::DataEventObservable<shared_ptr<TDCDataEvent>>
                 &tdcDataObservable,
             Observer::DataEventObservable<shared_ptr<EVRDataEvent>>
                 &evrDataObservable,
             Observer::DataEventObservable<PixelDataEvent> &pixelDataObservable);

  ~DataParser(){};

  int parse(const char *buffer, unsigned int size);

  struct Counters &Stats;

  Observer::DataEventObservable<shared_ptr<TDCDataEvent>> &tdcDataObservable;
  Observer::DataEventObservable<shared_ptr<EVRDataEvent>> &evrDataObservable;
  Observer::DataEventObservable<PixelDataEvent> &pixelDataObservable;
};

} // namespace Timepix3
