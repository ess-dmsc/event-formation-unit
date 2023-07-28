// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Timepix3 Modules
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>
#include <modules/timepix3/Counters.h>
#include <vector>

namespace Timepix3 {

// | 8b  |      12b        |     35b   |  4b   |  5b  |
// | 0x6 | Trigger counter | Timestamp | Stamp | Resv |


#define TYPE_MASK   0xF000000000000000
#define TYPE_OFFS 60

//pixel type data
#define PIXEL_DCOL_MASK   0x0FE0000000000000
#define PIXEL_SPIX_MASK   0x001F800000000000
#define PIXEL_PIX_MASK    0x0000700000000000
#define PIXEL_TOA_MASK    0x00000FFFC0000000
#define PIXEL_TOT_MASK    0x000000003FF00000
#define PIXEL_FTOA_MASK   0x00000000000F0000
#define PIXEL_SPTIME_MASK 0x000000000000FFFF
#define PIXEL_DCOL_OFFSET 52
#define PIXEL_SPIX_OFFSET 45
#define PIXEL_PIX_OFFSET  44
#define PIXEL_TOA_OFFSET  28
#define PIXEL_TOT_OFFSET  20
#define PIXEL_FTOA_OFFSET 16

//TDC type data
#define TDC_TYPE_MASK             0x0F00000000000000
#define TDC_TRIGGERCOUNTER_MASK   0x00FFF00000000000
#define TDC_TIMESTAMP_MASK        0x00000FFFFFFFFE00
#define TDC_STAMP_MASK            0x00000000000001E0
#define TDC_TYPE_OFFSET           56
#define TDC_TRIGGERCOUNTER_OFFSET 44
#define TDC_TIMESTAMP_OFFSET      9
#define TDC_STAMP_OFFSET          5

//Global Timestamp type data
#define GLOBAL_TIMESTAMP_MASK 0x00FFFFFFFFFFFF00
#define GLOBAL_STAMP_MASK     0x00000000000000F0
#define GLOBAL_TIMESTAMP_OFFSET 8
#define GLOBAL_STAMP_OFFSET 4

class DataParser {
public:
  const unsigned int MaxReadoutsInPacket{500};

  struct Timepix3PixelReadout {
    uint16_t Dcol;
    uint16_t Spix;
    uint8_t Pix;
    uint16_t ToA;
    uint16_t ToT;
    uint8_t FToA;
    uint16_t SpidrTime;
  }; // WARNING timepix3 readouts aren't packed like other detector readouts
     // each variable has an odd number of bits, and need to be extracted
     // with bitwise operations, this isn't like other detectors

  struct Timepix3TDCReadout {
    uint8_t Type;
    uint16_t TriggerCounter;
    uint64_t Timestamp;
    uint8_t Stamp;
  };

  struct Timepix3GlobalTimeReadout {
    uint64_t Timestamp;
    uint8_t Stamp;
  };

  struct EVRTimeReadout {
    uint8_t Type;
    uint8_t Unused;
    uint16_t Unused2;
    uint32_t Counter;
    uint32_t PulseTimeSeconds;
    uint32_t PulseTimeNanoSeconds;
    uint32_t PrevPulseTimeSeconds;
    uint32_t PrevPulseTimeNanoSeconds;
  } __attribute__((__packed__));

  DataParser(struct Counters &counters) : Stats(counters) {
    PixelResult.reserve(MaxReadoutsInPacket);
  };
  ~DataParser(){};

  //
  int parse(const char *buffer, unsigned int size);

  // To be iterated over in processing thread
  std::vector<struct Timepix3PixelReadout> PixelResult;

  uint64_t LastEVRTime;
  uint64_t LastTDCTime;

  struct Counters &Stats;
};
} // namespace Timepix3
