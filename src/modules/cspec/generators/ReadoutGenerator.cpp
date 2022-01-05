// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <math.h>
#include <time.h>
#include <modules/cspec/generators/ReadoutGenerator.h>
#include <stdexcept>



Cspec::ReadoutGenerator::ReadoutGenerator(uint8_t *BufferPtr, uint16_t MaxPayloadSize,
  uint32_t InitialSeqNum, bool Randomise)
  : Buffer(BufferPtr)
  , BufferSize(MaxPayloadSize)
  , SeqNum(InitialSeqNum)
  , Random(Randomise) { }


uint16_t Cspec::ReadoutGenerator::makePacket(
  uint8_t Type, uint16_t NumReadouts, uint8_t Rings,
  uint32_t TicksBtwReadouts, uint32_t TicksBtwEvents) {
  TimeBtwReadout = TicksBtwReadouts;
  TimeBtwEvents = TicksBtwEvents;
  generateHeader(Type, NumReadouts);
  generateData(Rings, NumReadouts);
  finishPacket();
  return DataSize;
}

void Cspec::ReadoutGenerator::generateHeader(uint8_t Type, uint16_t NumReadouts) {

  DataSize = HeaderSize + NumReadouts * VMM3DataSize;
  if (DataSize >= BufferSize) {
    throw std::runtime_error("Too many readouts for buffer size");
  }

  TimeHigh = time(NULL);

  memset(Buffer, 0, BufferSize);
  auto Header = (ESSReadout::Parser::PacketHeaderV0 *)Buffer;

  Header->CookieAndType = (Type << 24) + 0x535345;
  Header->Padding0 = 0;
  Header->Version = 0;
  // Header->OutputQueue = 0x00;

  Header->TotalLength = DataSize;
  Header->SeqNum = SeqNum;
  Header->PulseHigh = TimeHigh;
  Header->PulseLow = TimeLowOffset;
  Header->PrevPulseHigh = TimeHigh;
  Header->PrevPulseLow = PrevTimeLowOffset;
}


void Cspec::ReadoutGenerator::generateData(uint8_t Rings, uint16_t NumReadouts) {
  auto DP = (uint8_t *)Buffer;
  DP += HeaderSize;

  double XChannel = 384;
  double YChannel = 2240;

  uint32_t TimeLow = TimeLowOffset + TimeToFirstReadout;
  for (auto Readout = 0; Readout < NumReadouts; Readout++) {
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;
    ReadoutData->RingId = (Readout / 10) % Rings;
    ReadoutData->FENId = 0x01;
    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;
    ReadoutData->VMM = Readout & 0x3;
    ReadoutData->OTADC = 1000;

    XChannel = 50;
    YChannel = 50;

    if ((Readout % 2) == 0) {
      ReadoutData->Channel = YChannel;
    } else {
      ReadoutData->Channel = XChannel;
    }
    DP += VMM3DataSize;
    if ((Readout % 2) == 0) {
      TimeLow += TimeBtwReadout;
    } else {
      TimeLow += TimeBtwEvents;
    }
  }
}


void Cspec::ReadoutGenerator::finishPacket() {
  SeqNum++; // ready for next packet

  // if doing fuzzing, fuzz up to one field in header & up to 20 fields in data
  if (Random) {
    Fuzzer.fuzz8Bits(Buffer, HeaderSize, 1);
    Fuzzer.fuzz8Bits(Buffer + HeaderSize, DataSize - HeaderSize, 20);
  }
}


// GCOVR_EXCL_STOP
