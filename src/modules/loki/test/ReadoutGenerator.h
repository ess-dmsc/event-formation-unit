/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial LoKI readouts with variable number
/// of sections and data elements per section.
//===----------------------------------------------------------------------===//

#pragma once

#include <loki/readout/DataParser.h>
#include <random>

class ReadoutGenerator {
public:

  ReadoutGenerator() {
    std::random_device rd;
    gen = new std::mt19937(rd());
  }

  uint32_t random32() {
    return dist(*gen);
  }

  uint16_t random16() {
    return dist(*gen) & 0xFFFF;
  }

  uint8_t random8() {
    return dist(*gen) & 0xFF;
  }

/// \brief Fill out specified buffer with LoKI readouts
  uint16_t lokiReadoutDataGen(bool Randomise, uint16_t DataSections, uint16_t DataElements, uint8_t Rings,
       uint8_t * Buffer, uint16_t MaxSize, uint32_t SeqNum);

private:
  std::mt19937 * gen;
  std::uniform_int_distribution<uint32_t> dist;
};
