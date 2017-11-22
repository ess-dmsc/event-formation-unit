//
// Created by soegaard on 8/25/17.
//

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
