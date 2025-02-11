// Copyright (C) 2023 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Timepix3 Modules
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <dataflow/DataObserverTemplate.h>
#include <dto/TimepixDataTypes.h>
#include <modules/timepix3/Counters.h>

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
#define PIXEL_TOA_OFFSET    30
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

class DataParser
    : public Observer::DataEventObservable<timepixReadout::TDCReadout>,
      public Observer::DataEventObservable<timepixReadout::EVRReadout>,
      public Observer::DataEventObservable<timepixReadout::PixelReadout> {
public:
  const unsigned int MaxReadoutsInPacket{500};

  DataParser(Counters &counters);

  ~DataParser(){};

  int parse(const char *buffer, unsigned int size);

  struct Counters &Stats;

private:
  // Const expression
  static constexpr uint8_t PIXEL_READOUT_TYPE_CONST = 11;
  static constexpr uint8_t TDC_READOUT_TYPE_CONST = 6;

  static constexpr uint8_t TDC1_RISING_CONST = 15;
  static constexpr uint8_t TDC1_FALLING_CONST = 10;
  static constexpr uint8_t TDC2_RISING_CONST = 14;
  static constexpr uint8_t TDC2_FALLING_CONST = 11;
};

} // namespace Timepix3
