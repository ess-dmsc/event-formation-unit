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
    uint8_t packet_type;
    uint16_t trigger_counter;
    uint32_t timestamp;
    uint8_t stamp;
  };

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
