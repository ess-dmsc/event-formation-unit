// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for fuzzing a data buffer: writing random data at random
/// locations according to specifications.
//===----------------------------------------------------------------------===//

#include <common/DataFuzzer.h>

void DataFuzzer::fuzz8Bits(void * Buffer, uint16_t BufferLength, uint16_t Repeats)  {
  uint16_t Reps = randomInterval(0, Repeats);
  for (uint16_t i = 0; i < Reps; i++) {
    uint16_t Offset = randomInterval(0, BufferLength - 2); // make room for last 8 bits
    uint8_t Value = random8();
    *((uint8_t*)Buffer + Offset) = Value;
  }
}
