// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS VMM3 readout parser
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>
#include <cinttypes>
#include <vector>

namespace ESSReadout {

struct VMM3ParserStats {
  int64_t ErrorSize{0};
  int64_t ErrorRing{0};
  int64_t ErrorFEN{0};
  int64_t ErrorDataLength{0};
  int64_t ErrorTimeFrac{0};
  int64_t ErrorBC{0};
  int64_t ErrorADC{0};
  int64_t ErrorVMM{0};
  int64_t ErrorChannel{0};
  int64_t Readouts{0};
  int64_t CalibReadouts{0};
  int64_t DataReadouts{0};
  int64_t OverThreshold{0};
};


class VMM3Parser {
public:
  const unsigned int MaxRingId{23}; // Physical rings
  const unsigned int MaxFENId{23};
  const unsigned int MaxReadoutsInPacket{500};

  // From VMM3 ICD
  // https://project.esss.dk/owncloud/index.php/s/Nv17qiLWwOTkbzE
  // Since there will always be a single readout per header we combine
  // the two fields into one.
  #define VMM3DATASIZE 20
  struct VMM3Data {
    uint8_t RingId;
    uint8_t FENId;
    uint16_t DataLength;
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint16_t BC;
    uint16_t OTADC;
    uint8_t GEO;
    uint8_t TDC;
    uint8_t VMM;
    uint8_t Channel;
  } __attribute__((packed));

  static_assert(sizeof(VMM3Parser::VMM3Data) == (VMM3DATASIZE),
                "Wrong header size (update assert or check packing)");

  VMM3Parser() {
    Result.reserve(MaxReadoutsInPacket);
  };
  ~VMM3Parser(){};

  //
  int parse(Parser::PacketDataV0 & PacketData);

  // To be iterated over in processing thread
  std::vector<struct VMM3Data> Result;

  struct VMM3ParserStats Stats;

private:
  const uint16_t DataLength{20};
  const uint16_t MaxBCValue{4095};
  const uint16_t MaxADCValue{1023};
  const uint16_t MaxVMMValue{15};
  const uint16_t MaxChannelValue{63};
  const uint16_t OverThresholdMask{0x8000};
  const uint16_t ADCMask{0x7fff};
};
} // namespace ESSReadout
