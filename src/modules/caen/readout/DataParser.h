// Copyright (C) 2019-2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Caen Modules
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>
#include <modules/caen/Counters.h>
#include <vector>

namespace Caen {

class DataParser {
public:
  const unsigned int MaxRingId{11};
  const unsigned int MaxFENId{23};
  const unsigned int MaxReadoutsInPacket{500};

  struct CaenReadout {
    uint8_t RingId;
    uint8_t FENId;
    uint16_t DataLength;
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint8_t unused;
    uint8_t TubeId;
    uint16_t DataSeqNum;
    int16_t AmpA;
    int16_t AmpB;
    int16_t AmpC;
    int16_t AmpD;
  } __attribute__((__packed__));

  static_assert(sizeof(CaenReadout) == 24, "Caen readout header length error");

  DataParser(struct Counters &counters) : Stats(counters) {
    Result.reserve(MaxReadoutsInPacket);
  };
  ~DataParser(){};

  //
  int parse(const char *buffer, unsigned int size);

  // To be iterated over in processing thread
  std::vector<struct CaenReadout> Result;

  struct Counters &Stats;
};
} // namespace Caen
