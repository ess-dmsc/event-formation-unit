// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of DREAM
//===----------------------------------------------------------------------===//

#pragma once

#include <readout/ReadoutParser.h>
#include <dream/Counters.h>
#include <vector>

namespace Jalousie {

class DataParser {
public:
  const unsigned int MaxRingId{11};
  const unsigned int MaxFENId{23};
  const unsigned int MaxReadoutsInPacket{500};

  struct DreamReadout //
  {
    uint32_t Tof;
    uint16_t unused;
    uint8_t Module;
    uint8_t Sumo;
    uint8_t Strip;
    uint8_t Wire;
    uint8_t Segment;
    uint8_t Counter;
  } __attribute__((__packed__));

  static_assert(sizeof(DreamReadout) == 12, "DREAM readout header length error");

  DataParser(struct Counters & counters) : Stats(counters){
    Result.reserve(MaxReadoutsInPacket);
  };
  ~DataParser(){};

  //
  int parse(const char *buffer, unsigned int size);

  //
  struct ParsedData {
    uint8_t RingId;
    uint8_t FENId;
    std::vector<DreamReadout> Data;
  };

  // To be iterated over in processing thread
  std::vector<struct ParsedData> Result;

  struct Counters & Stats;
};
}
