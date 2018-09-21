/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for JADAQ
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

class DataParser {
public:
  enum error { OK = 0, ESIZE};

  struct __attribute__ ((__packed__)) Header // 32 bytes
  {
      uint64_t runID;
      uint64_t globalTime;
      uint32_t digitizerID;
      uint16_t elementType;
      uint16_t numElements;
      uint16_t version;
      uint8_t __pad[6];
  };

  struct __attribute__ ((__packed__)) ListElement422
  {
      uint32_t localTime;
      uint16_t channel;
      uint16_t adcValue;
  };

  DataParser() {};

  int parse(const char * /*void **/ buffer, unsigned int size);

  struct Header * MBHeader{nullptr};
  struct ListElement422 * MBData{nullptr};

  struct Stats {
    uint64_t error_bytes{0};
  } Stats;
};
