/** Copyright (C) 2019 European Spallation Source ERIC */

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <loki/readout/DataParser.h>

using namespace Loki;

/// in benchmark tests
uint16_t lokiReadoutDataGen(uint16_t DataSections, uint16_t DataElements, uint8_t Rings,
     uint8_t * Buffer, uint16_t MaxSize) {

  auto DataSize = 28 + DataSections * (4 + DataElements * 20);
  if (DataSize > MaxSize) {
    printf("Too much data for buffer\n");
    return 0;
  }

  //printf("Write header (28 bytes)\n");
  memset(Buffer, 0, MaxSize);
  auto DP = (uint8_t *)Buffer;
  //printf("Buffer pointer %p\n", (void *)Buffer);
  auto Header = (ReadoutParser::PacketHeaderV0 *)DP;
  Header->CookieVersion = 0x00535345;
  Header->TypeSubType = 0x30;
  //Header->OutputQueue = 0x00;
  Header->TotalLength = DataSize;
  uint8_t RingCount{0};
  DP += 28;
  for (auto Section = 0; Section < DataSections; Section++) {
    auto DataHeader = (ReadoutParser::DataHeader *)DP;
    DataHeader->RingId = RingCount % Rings;
    DataHeader->FENId = 0x00;
    DataHeader->DataLength = sizeof(ReadoutParser::DataHeader) +
       DataElements * sizeof(DataParser::LokiReadout);
    assert(DataHeader->DataLength == 4 + 20 * DataElements);
    RingCount++;
    //printf("  Data Header %u @ %p (4 bytes)\n", Section, (void *)DP);
    DP += sizeof(ReadoutParser::DataHeader);
    for (auto Element = 0; Element < DataElements; Element++) {
      auto DataBlock = (DataParser::LokiReadout *)DP;
      DataBlock->TimeLow = 100;
      DataBlock->FpgaAndTube = Element % 8;
      DataBlock->AmpA = DataElements - Element;
      DataBlock->AmpB = Element + 1;
      DataBlock->AmpC = Element + 1;
      DataBlock->AmpD = Element + 1;
      //printf("    Data Element %u @ %p (20 bytes)\n", Element, (void *)DP);
      assert(sizeof(DataParser::LokiReadout) == 20);
      DP += sizeof(DataParser::LokiReadout);
    }
  }
  // for (uint16_t i = 0; i < DataSize; i++) {
  //   if (i % 4 == 0) {
  //     printf("\n");
  //   }
  //   printf("%02x ", Buffer[i]);
  // }
  // printf("\n");
  return DataSize;
}
