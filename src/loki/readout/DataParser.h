/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of LoKI
//===----------------------------------------------------------------------===//

#pragma once

#include <readout/Readout.h>
#include <vector>

namespace Loki {

class DataParser {
public:
  enum error { OK = 0, EHEADER, ESIZE};

  const unsigned int MaxRingId{11};
  const unsigned int MaxFENId{23};

  struct  Header // 32 bytes
  {
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint16_t FpgaAndTube;
    uint16_t ADC;
    uint16_t AmplitudeA;
    uint16_t AmplitudeB;
    uint16_t AmplitudeC;
    uint16_t AmplitudeD;
  } __attribute__ ((__packed__));

  DataParser() { };
  ~DataParser(){ };

  int parse(const char * buffer, unsigned int size);

  struct {
    int64_t Readouts{0};
    int64_t Headers{0};
    int64_t ErrorHeaders{0};
    int64_t ErrorBytes{0};
  } Stats;
};

}
