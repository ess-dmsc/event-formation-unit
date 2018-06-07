//
// Created by soegaard on 8/25/17.
//

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
      uint16_t adcValue;
      uint16_t channel;
  };

  DataParser() {};

  int parse(const char * /*void **/ buffer, unsigned int size);

  struct Header * mbheader{nullptr};
  struct ListElement422 * mbdata{nullptr};

  struct stats{
    uint64_t error_bytes{0};
  } stats;
};
