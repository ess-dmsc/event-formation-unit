// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS VMM3 readout parser
///
//===----------------------------------------------------------------------===//

#pragma once

#include <multiblade/Counters.h>
#include <cinttypes>
#include <vector>

// struct readoutdatastat_t {
//   int64_t ErrorSize{0};
//   int64_t ErrorRing{0};
//   int64_t ErrorFEN{0};
//   int64_t CalibReadout{0};
//   int64_t DataReadout{0};
// };


class VMM3Parser {
public:
  const unsigned int MaxRingId{11};
  const unsigned int MaxFENId{23};
  const unsigned int MaxReadoutsInPacket{500};

  // From VMM3 ICD
  // https://project.esss.dk/owncloud/index.php/s/Nv17qiLWwOTkbzE
  // Since there will always be a single readout per header we combine
  // the two fields into one.
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

  static_assert(sizeof(VMM3Parser::VMM3Data) == (20),
                "Wrong header size (update assert or check packing)");

  VMM3Parser(struct Counters &counters) : Stats(counters) {
    Result.reserve(MaxReadoutsInPacket);
  };
  ~VMM3Parser(){};

  //
  int parse(const char *buffer, unsigned int size);

  // To be iterated over in processing thread
  std::vector<struct VMM3Data> Result;

  struct Counters &Stats;
};
