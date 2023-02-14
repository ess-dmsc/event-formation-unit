// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of TREX
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>
#include <trex/Counters.h>

#include <vector>

namespace Trex {

class DataParser {
public:
  const unsigned int MaxRingId{11};
  const unsigned int MaxFENId{23};
  const unsigned int MaxReadoutsInPacket{500};

  struct TREXReadout //
  {
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint8_t unused;
    uint8_t TubeId;
    uint16_t DataSeqNum;

  } __attribute__((__packed__));

  static_assert(sizeof(TREXReadout) == 20,
                "TREX readout header length error");

  DataParser(struct Counters &counters) : Stats(counters) {
    Result.reserve(MaxReadoutsInPacket);
  };
  ~DataParser(){};

  //
  int parse(const char *buffer, unsigned int size);

  //
  struct ParsedData {
    uint8_t RingId;
    uint8_t FENId;
    TREXReadout Data;
  };

  // To be iterated over in processing thread
  std::vector<struct ParsedData> Result;

  struct Counters &Stats;
  uint32_t HeaderCounters[16][16]; // {ring,fen} counters
};
} // namespace Trex
