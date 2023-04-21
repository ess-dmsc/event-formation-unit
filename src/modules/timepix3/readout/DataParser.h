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
#define PIXEL_DCOL_OFFS 52
#define PIXEL_SPIX_OFFS 45
#define PIXEL_PIX_OFFS 44
#define PIXEL_TOA_OFFS 28
#define PIXEL_TOT_OFFS 20
#define PIXEL_FTOA_OFFS 16

//


class DataParser {
public:
  const unsigned int MaxReadoutsInPacket{500};

  struct Timepix3PixelReadout {
    uint16_t dcol;
    uint16_t spix;
    uint8_t pix;
    uint16_t ToA;
    uint16_t ToT;
    uint8_t FToA;
    uint16_t spidr_time;
  }; // WARNING timepix3 readouts aren't packed like other detector readouts
     // each variable has an odd number of bits, and need to be extracted
     // with bitwise operations, this isn't like other detectors

  struct Timepix3TDCReadout {
    uint8_t type;
    uint16_t trigger_counter;
    uint64_t timestamp;
    uint8_t stamp;
  };

  struct Timepix3GlobalTimeReadout {
    uint64_t timestamp;
    uint8_t stamp;
  };

  struct EVRTimeReadout {
    uint8_t type;
    uint8_t unused;
    uint16_t unused2;
    uint32_t counter;
    uint32_t pulseTimeSeconds;
    uint32_t pulseTimeNanoSeconds;
    uint32_t prevPulseTimeSeconds;
    uint32_t prevPulseTimeNanoSeconds;
  } __attribute__((__packed__));

  DataParser(struct Counters &counters) : Stats(counters) {
    PixelResult.reserve(MaxReadoutsInPacket);
  };
  ~DataParser(){};

  //
  int parse(const char *buffer, unsigned int size);

  // To be iterated over in processing thread
  std::vector<struct Timepix3PixelReadout> PixelResult;

  uint64_t lastEVRTime;
  uint64_t lastTDCTime;

  struct Counters &Stats;
};
} // namespace Timepix3
