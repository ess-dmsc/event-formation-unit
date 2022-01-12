/* Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of CSPEC
//===----------------------------------------------------------------------===//

#pragma once

#include <cspec/Counters.h>
#include <common/readout/ess/Parser.h>
#include <vector>

namespace Cspec {

class DataParser {
public:
  const unsigned int MaxRingId{11};
  const unsigned int MaxFENId{23};
  const unsigned int MaxReadoutsInPacket{500};

  struct CSPECReadout //
  {
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint8_t unused;
    uint8_t TubeId;
    uint16_t DataSeqNum;

  } __attribute__((__packed__));

  static_assert(sizeof(CSPECReadout) == 20, "CSPEC readout header length error");

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
    CSPECReadout Data;
  };

  // To be iterated over in processing thread
  std::vector<struct ParsedData> Result;

  struct Counters &Stats;
  uint32_t HeaderCounters[16][16]; // {ring,fen} counters
};
} // namespace Cspec
