/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for JADAQ
//===----------------------------------------------------------------------===//

#pragma once

#include <multiblade/caen/Readout.h>
#include <vector>

namespace Multiblade {

/// \todo consider using the real values  and using
/// htons() in the parser
// 0x0102 (1.3) byteswapped
constexpr uint16_t Version = 0x0301;
// 0x0100 byteswapped)
constexpr uint16_t ElementType = 0x0001;

class DataParser {
public:
  enum error { OK = 0, ESIZE, EHEADER };

  struct __attribute__ ((__packed__)) Header // 32 bytes
  {
    uint64_t runID;
    uint64_t globalTime;
    uint32_t digitizerID;
    uint16_t elementType;
    uint16_t numElements;
    uint16_t version;
    uint32_t seqNum;
    uint8_t __pad[2];
  };

  struct __attribute__ ((__packed__)) ListElement422 {
    uint32_t localTime;
    uint16_t channel;
    uint16_t adcValue;
  };

  DataParser() {}

  int parse(const char * /*void **/ buffer, unsigned int size);

  struct Stats {
    uint64_t error_version{0};
    uint64_t error_bytes{0};
    uint64_t seq_errors{0};
  } Stats;

  Readout prototype;
  std::vector<Readout> readouts;

  struct Header *MBHeader{nullptr};
  struct ListElement422 *Data{nullptr};

  uint32_t PreviousSeqNum{0};
};

}
