// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief reader for raw loki detector data
///
/// Raw data format gotten from Davide Raspino at STFC
///
//===----------------------------------------------------------------------===//

#pragma once

#include <loki/readout/DataParser.h>
#include <cinttypes>
#include <string>

class LokiReader {
public:

  // From Davide's readout system
  struct raw_data_t {
    uint32_t cookie;
    uint32_t tof1;
    uint32_t tof2;
    uint16_t dump;
    uint8_t tube;
    uint8_t fpga;
    uint16_t a;
    uint16_t b;
    uint16_t c;
    uint16_t d;
  } __attribute__((__packed__));
  static_assert(sizeof(struct raw_data_t) == 24, "wrong packing");

  //
  LokiReader(std::string file);

  // Read 4 bytes into val, return bytes read or -1
  int read32(uint32_t & val);

  // Read 2 bytes into val, return bytes read or -1
  int read16(uint16_t & val);

  // Read 1 bytes into val, return bytes read or -1
  int read8(uint8_t & val);

  // Skip n bytes
  void skip(int n);

  // Read a LokiReadout struct, return bytes read or -1
  int readReadout(struct Loki::DataParser::LokiReadout & Readout);

private:
  std::string filename;
  int fd;
  uint64_t readoutReads{0};
};
