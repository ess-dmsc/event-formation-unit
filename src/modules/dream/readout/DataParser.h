// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of DREAM
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>
#include <modules/dream/Counters.h>
#include <vector>

namespace Dream {

class DataParser {
public:
  const unsigned int MaxRingId{11};
  const unsigned int MaxFENId{12};
  const unsigned int MaxReadoutsInPacket{500};

  struct DreamReadout {
    uint8_t RingId;
    uint8_t FENId;
    uint16_t DataLength;
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint8_t OM;
    uint8_t Unused;
    uint8_t Cathode;
    uint8_t Anode;
  } __attribute__((__packed__));

  static_assert(sizeof(DreamReadout) == 16,
                "DREAM readout header length error");

  DataParser(struct Counters &counters) : Stats(counters) {
    Result.reserve(MaxReadoutsInPacket);
  };
  ~DataParser(){};

  //
  int parse(const char *buffer, unsigned int size);

  // To be iterated over in processing thread
  std::vector<struct DreamReadout> Result;

  struct Counters &Stats;
};
} // namespace Dream
