// Copyright (C) 2019 - 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial LoKI readouts with variable number
/// of sections and data elements per section.
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <loki/test/ReadoutGenerator.h>

using namespace Loki;

uint16_t ReadoutGenerator::lokiReadoutDataGen(bool Randomise, uint16_t DataSections, uint16_t DataElements, uint8_t Rings,
     uint8_t * Buffer, uint16_t MaxSize, uint32_t SeqNum) {

  uint16_t DataSize = HeaderSize + DataSections * (4 + DataElements * 20);
  if (DataSize > MaxSize) {
    printf("Too much data for buffer. DataSize: %u, MaxSize: %u\n", DataSize, MaxSize);
    return 0;
  }

  memset(Buffer, 0, MaxSize);
  auto DP = (uint8_t *)Buffer;
  auto Header = (ReadoutParser::PacketHeaderV0 *)DP;

  Header->CookieAndType = 0x30535345;
  Header->Padding0 = 0;
  Header->Version = 0;
  //Header->OutputQueue = 0x00;

  Header->TotalLength = DataSize;
  Header->SeqNum = SeqNum;

  uint8_t RingCount{0};

  DP += HeaderSize;
  for (auto Section = 0; Section < DataSections; Section++) {
    auto DataHeader = (ReadoutParser::DataHeader *)DP;
    DataHeader->RingId = RingCount % Rings;
    DataHeader->FENId = 0x00;
    DataHeader->DataLength = DataHeaderSize +
       DataElements * LokiDataSize;
    assert(DataHeader->DataLength == 4 + 20 * DataElements);
    RingCount++;
    DP += DataHeaderSize;

    for (auto Element = 0; Element < DataElements; Element++) {
      auto DataBlock = (DataParser::LokiReadout *)DP;
      DataBlock->TimeLow = 100;
      DataBlock->TubeId = Element % 8;
      DataBlock->AmpA = DataElements - Element;
      DataBlock->AmpB = Element + 1;
      DataBlock->AmpC = Element + 1;
      DataBlock->AmpD = Element + 1;
      DP += LokiDataSize;
    }
  }

  // if doing fuzzing, fuzz up to one field in header & up to 20 fields in data
  if (Randomise) {
    Fuzzer.fuzz8Bits(Buffer, HeaderSize, 1);
    Fuzzer.fuzz8Bits(Buffer + HeaderSize, DataSize - HeaderSize, 20);
  }

  return DataSize;
}
