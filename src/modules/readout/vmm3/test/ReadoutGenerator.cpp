// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <readout/vmm3/test/ReadoutGenerator.h>

// GCOVR_EXCL_START

uint16_t ReadoutGenerator::vmm3ReadoutDataGen(
  uint8_t *Buffer, uint16_t MaxSize, bool Randomise,
  uint8_t Type, uint32_t SeqNum, uint8_t Rings, uint16_t NumReadouts) {

  uint16_t DataSize = HeaderSize + NumReadouts * sizeof(VMM3Parser::VMM3Data);
  if (DataSize > MaxSize) {
    printf("Too much data for buffer. DataSize: %u, MaxSize: %u\n", DataSize,
           MaxSize);
    return 0;
  }

  memset(Buffer, 0, MaxSize);
  auto DP = (uint8_t *)Buffer;
  auto Header = (ReadoutParser::PacketHeaderV0 *)DP;

  Header->CookieAndType = 0x535345 + (Type << 24);
  Header->Padding0 = 0;
  Header->Version = 0;
  // Header->OutputQueue = 0x00;

  Header->TotalLength = DataSize;
  Header->SeqNum = SeqNum;

  uint8_t RingCount{0};

  DP += HeaderSize;
  for (auto Readout = 0; Readout < NumReadouts; Readout++) {
    auto ReadoutData = (VMM3Parser::VMM3Data *)DP;
    ReadoutData->RingId = RingCount % Rings;
    ReadoutData->FENId = 0x01;
    ReadoutData->DataLength = sizeof(VMM3Parser::VMM3Data);
    assert(ReadoutData->DataLength == 20);
    RingCount++;
    ReadoutData->TimeHigh = SeqNum;
    ReadoutData->TimeLow = 100;
    DP += VMM3DataSize;
  }

  // if doing fuzzing, fuzz up to one field in header & up to 20 fields in data
  if (Randomise) {
    Fuzzer.fuzz8Bits(Buffer, HeaderSize, 1);
    Fuzzer.fuzz8Bits(Buffer + HeaderSize, DataSize - HeaderSize, 20);
  }

  return DataSize;
}
// GCOVR_EXCL_STOP
