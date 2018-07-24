/// Copyright (C) 2017-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// /brief parser for multiblade data readout using Caen digitizers
//===----------------------------------------------------------------------===//

#pragma once
#include <cstdint>
#include <vector>

struct datapoint {
  uint8_t digi;
  uint8_t chan;
  uint16_t adc;
  uint32_t time;
};

class MBData {
public:
  MBData();

  unsigned int receive(const char * /*void **/ buffer, unsigned int size);

  std::vector<datapoint> data;

private:
};
