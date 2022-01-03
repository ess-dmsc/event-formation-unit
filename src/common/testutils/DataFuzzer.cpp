// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for fuzzing a data buffer: writing random data at random
/// locations according to specifications.
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cassert>
#include <common/testutils/DataFuzzer.h>


void DataFuzzer::fuzz8Bits(void * Buffer, uint16_t BufferLength, uint16_t MaxRepeats)  {
  // randomInterval(0, N) yields values from 0, N - 1. Hence the + 1
  uint16_t Repeats = randomInterval(0, MaxRepeats + 1);
  //printf("fuzz8Bits %u repeats (%u possible)\n", Repeats, MaxRepeats);
  for (uint16_t i = 0; i < Repeats; i++) {
    uint16_t Offset = randomInterval(0, BufferLength);
    uint8_t Value = random8();
    //printf("Offset: %u, value: %u\n", Offset, Value);
    assert(Offset < BufferLength);
    *((uint8_t*)Buffer + Offset) = Value;
  }
}

// GCOVR_EXCL_STOP
