// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief reader for Bifrost data
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>

class BifrostDatReader {
public:
  struct dat_data_t {
    uint32_t timehi;
    uint32_t timelow;
    uint8_t fiber;
    uint16_t unused;
    uint8_t tube;
    uint16_t ampl_a;
    uint16_t ampl_b;
  } __attribute__((__packed__));
  static_assert(sizeof(struct dat_data_t) == 16, "wrong packing");

  struct udp_data_t {
    uint32_t timehi;
    uint32_t timelow;
    uint8_t OM;
    uint8_t tube;
    uint16_t unused2;
    uint16_t ampl_a;
    uint16_t ampl_b;
    uint32_t unused3;
  } __attribute__((__packed__));
  static_assert(sizeof(struct udp_data_t) == 20, "wrong packing");

  //
  BifrostDatReader(std::string file, bool Verbose);

  // Read a DreamReadout struct, return bytes read, 0 if line is
  // ignored, or -1 upon error/end
  int readReadout(struct dat_data_t &reaout);

private:
  std::string filename;
  int FileDescriptor{-1};
  bool Verbose{false};
  uint64_t Readouts{0};
};
