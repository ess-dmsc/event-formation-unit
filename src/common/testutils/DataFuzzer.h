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
  void fuzz8Bits(void *Buffer, uint16_t BufferLength, uint16_t Repeats);

  /// \brief generate random 8 bit values
  uint8_t random8() { return IntDist(Generator) & 0xFF; }

  /// \brief generate random 16 bit values
  uint16_t random16() { return IntDist(Generator) & 0xFFFF; }

  /// \brief Generate random ints in the interval [Begin ; End]
  uint16_t randomInterval(uint16_t Begin, uint16_t End) {
    return Begin + RealDist(Generator) * (End - Begin);
  }

  /// \brief Generate a random integer in a range and filter out some according
  ///        the defined mask.
  /// \param Range the maximum number of integers generated random generator.
  ///        The generator will generate numbers from 0 to Range -1. Range
  ///        must be nonzero and <= 32
  /// \param Mask the (32 bit) bitmask of allowed numbers. Only numbers allowed
  ///        by the mask are generated. Must be nonzero.
  ///
  /// Constraints:
  /// 1) The number must belong to the interval 0 to (Range - 1)
  /// 2) The number, represented as a bit, must be accepted by the supplied mask
  ///
  /// Example:
  /// If Range is 12 then to generate only numbers 1, 2, 5, 6 Mask should be
  /// set as follows:
  /// Possible numbers: 11 10  9  8  7  6  5  4  3  2  1  0
  /// Binary Mask:       0  0  0  0  0  1  1  0  0  1  1  0
  /// Mask in HEX:      0x066
  ///
  /// Other examples:
  /// 1) If Mask is 0x03 (11 in binary), the only allowed values are 0
  /// and 1 corresponding to the 0'th and first bit.
  ///
  /// 2) Mask 0x09 would allow values 0 and 3, so randU8WithMask(16, 0x09)
  /// will return a random sequence from the set (0, 3) whereas
  /// randU8WithMask(8, 0x09) will only return 0's.
  uint8_t randU8WithMask(int Range, int Mask=0xFF) {
    if (Mask < 1) {
      return 0;
    }

      // Repeat until match
    while (true) {
      uint8_t Id = random8() % Range;
      int BitVal = 1 << Id;
      if (BitVal & Mask) {
        return Id;
      }
    }
  }

private:
  std::seed_seq SeedSequence{1, 2, 3, 4, 5};
  std::mt19937 Generator{SeedSequence};
  std::uniform_int_distribution<uint32_t> IntDist;
  std::uniform_real_distribution<double> RealDist{0.0, 1.0};
};
// GCOVR_EXCL_STOP
