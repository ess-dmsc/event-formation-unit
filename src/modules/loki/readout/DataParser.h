/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of LoKI
//===----------------------------------------------------------------------===//

#pragma once

#include <readout/ReadoutParser.h>
#include <loki/Counters.h>
#include <vector>

namespace Loki {

class DataParser {
public:
  const unsigned int MaxRingId{11};
  const unsigned int MaxFENId{23};
  const unsigned int MaxReadoutsInPacket{500};

  struct LokiReadout //
  {
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint8_t unused;
    uint8_t TubeId;
    uint16_t DataSeqNum;
    uint16_t AmpA;
    uint16_t AmpB;
    uint16_t AmpC;
    uint16_t AmpD;
  } __attribute__((__packed__));

  static_assert(sizeof(LokiReadout) == 20, "LoKI readout header length error");

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
    std::vector<LokiReadout> Data;
  };

  // To be iterated over in processing thread
  std::vector<struct ParsedData> Result;

  struct Counters & Stats;
  uint32_t HeaderCounters[12][24]; // {ring,fen} counters
};
}
