// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS TTLMonitor readout parser
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <vector>
#include <common/readout/ess/Parser.h>

namespace TTLMonitor {

struct ParserStats {
  int64_t ErrorSize{0};
  int64_t ErrorRing{0};
  int64_t ErrorFEN{0};
  int64_t ErrorDataLength{0};
  int64_t ErrorTimeFrac{0};
  int64_t ErrorADC{0};
  int64_t Readouts{0};
};

class Parser {
public:
  const unsigned int MaxRingId{23}; // Physical rings
  const unsigned int MaxFENId{23};
  const unsigned int MaxReadoutsInPacket{600};

// From TTLMon ICD
// TBD
#define DATASIZE 16
  struct Data {
    uint8_t RingId;
    uint8_t FENId;
    uint16_t DataLength;
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint8_t Pos;
    uint8_t Channel;
    uint16_t ADC;
  } __attribute__((packed));

  static_assert(sizeof(Parser::Data) == (DATASIZE),
                "Wrong header size (update assert or check packing)");

  Parser() { Result.reserve(MaxReadoutsInPacket); };

  ~Parser(){};

  //
  int parse(ESSReadout::Parser::PacketDataV0 &PacketData);

  // To be iterated over in processing thread
  std::vector<struct Data> Result;

  struct ParserStats Stats;

private:
  const uint16_t DataLength{16};
};
} // namespace TTLMonitor
