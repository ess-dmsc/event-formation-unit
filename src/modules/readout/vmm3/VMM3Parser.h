// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS VMM3 readout parser
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>

struct readoutstat_t {
  int64_t ErrorSize{0};
  int64_t ErrorRing{0};
  int64_t ErrorFEN{0};
  int64_t CalibReadout{0};
  int64_t DataReadout{0};
};


class VMM3Parser {
public:


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

};
