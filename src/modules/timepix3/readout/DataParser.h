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
#define TSTAMP_MASK 0x00000FFFFFFFFE00
#define STAMP_MASK 0x00000000000001E0
#define TSTAMP_OFFS 10 // 9?
#define STAMP_OFFS 8   // 5?

#define TYPE_MASK 0xF000000000000000
#define DCOL_MASK 0x0FE0000000000000
#define SPIX_MASK 0x001F800000000000
#define PIX_MASK 0x0000700000000000
#define TOA_MASK 0x00000FFFC0000000
#define TOT_MASK 0x000000003FF00000
#define FTOA_MASK 0x00000000000F0000
#define SPTIME_MASK 0x000000000000FFFF
#define TYPE_OFFS 60
#define DCOL_OFFS 52
#define SPIX_OFFS 45
#define PIX_OFFS 44
#define TOA_OFFS 28
#define TOT_OFFS 20
#define FTOA_OFFS 16

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
    uint32_t timestamp;
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
    uint32_t pulseTimeSeconds;
    uint32_t pulseTimeNanoSeconds;
    uint32_t prevPulseTimeSeconds;
    uint32_t prevPulseTimeNanoSeconds;
  } __attribute__((__packed__));

  DataParser(struct Counters &counters) : Stats(counters) {
    Result.reserve(MaxReadoutsInPacket);
  };
  ~DataParser(){};

  //
  int parse(const char *buffer, unsigned int size);

  // To be iterated over in processing thread
  std::vector<struct Timepix3PixelReadout> Result;

  struct Counters &Stats;
};
} // namespace Timepix3
