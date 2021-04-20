// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for fuzzing a data buffer: writing random data at random
/// locations according to specifications.
//===----------------------------------------------------------------------===//

#pragma once

#include <random>

class DataFuzzer {
public:

  DataFuzzer() {
    std::random_device RandomDevice;
    Generator = new std::mt19937(RandomDevice());
  }

  /// \brief overwrite buffer with a number of random 8-bit values
  void fuzz8Bits(void * Buffer, uint16_t BufferLength, uint16_t Repeats);

  /// \brief generate random 8 bit values
  uint8_t random8() { return Idist(*Generator) & 0xFF; }

  /// \brief generate random 16 bit values
  uint16_t random16() { return Idist(*Generator) & 0xFFFF; }

  /// \brief Generate random ints in the interval [Begin ; End]
  uint16_t randomInterval(uint16_t Begin, uint16_t End) {
    return Begin + Rdist(*Generator)*(End - Begin);
  }

private:
  std::mt19937 * Generator;
  std::uniform_int_distribution<uint32_t> Idist;
  std::uniform_real_distribution<double> Rdist{0.0, 1.0};
};
