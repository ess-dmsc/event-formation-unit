// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for fuzzing a data buffer: writing random data at random
/// locations according to specifications.
///
/// Meant for use in data generators
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <random>

class DataFuzzer {
public:

  DataFuzzer() = default;

  /// \brief overwrite buffer with a number of random 8-bit values
  void fuzz8Bits(void * Buffer, uint16_t BufferLength, uint16_t Repeats);


  /// \brief generate random 8 bit values
  uint8_t random8() { return IntDist(Generator) & 0xFF; }

  /// \brief generate random 16 bit values
  uint16_t random16() { return IntDist(Generator) & 0xFFFF; }

  /// \brief Generate random ints in the interval [Begin ; End[
  uint16_t randomInterval(uint16_t Begin, uint16_t End) {
    return Begin + RealDist(Generator)*(End - Begin);
  }

private:
  std::seed_seq SeedSequence{1, 2, 3, 4, 5};
  std::mt19937 Generator{SeedSequence};
  std::uniform_int_distribution<uint32_t> IntDist;
  std::uniform_real_distribution<double> RealDist{0.0, 1.0};
};
// GCOVR_EXCL_STOP
