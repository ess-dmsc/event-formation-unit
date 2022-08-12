// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief reader for raw bifrost detector data
///
/// Raw data format gotten from Paolo Mutti
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>

class R5560Reader {
public:
  // From Paolo's .dat file
  struct raw_data_t {
    uint32_t TimeHi;
    uint32_t TimeLo;
    uint8_t IDnFlags;
    uint8_t TubeCh;
    uint16_t unused16;
    uint16_t AmplA;
    uint16_t AmplB;
    uint32_t unused32;
  } __attribute__((__packed__));
  static_assert(sizeof(struct raw_data_t) == 20, "wrong packing");

  //
  R5560Reader(std::string file);

  //
  ~R5560Reader();

  // Read a R5560Readout struct, return bytes read, 0 if line is
  // ignored, or -1 upon error/end
  int readReadout(struct raw_data_t &readout);

private:
  std::string filename;
  int fd;       // file descriptor
  int readouts; // number of readouts in file
};
