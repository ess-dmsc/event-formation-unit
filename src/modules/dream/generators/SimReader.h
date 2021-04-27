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

#include <cinttypes>
#include <string>

class DreamSimReader {
public:
  // From Irina's .txt file
  struct sim_data_t {
    uint32_t tof;
    uint16_t unused;
    uint8_t module;
    uint8_t sumo;
    uint8_t strip;
    uint8_t wire;
    uint8_t segment;
    uint8_t counter;
  } __attribute__((__packed__));
  static_assert(sizeof(struct sim_data_t) == 12, "wrong packing");

  //
  DreamSimReader(std::string file);

  // Read a DreamReadout struct, return bytes read, 0 if line is
  // ignored, or -1 upon error/end
  int readReadout(struct sim_data_t &reaout);

private:
  std::string filename;
  std::ifstream *infile;
  uint32_t lines{0};
};
