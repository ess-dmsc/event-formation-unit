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

  uint32_t TimeLow = TimeLowOffset + TimeToFirstReadout;
  for (auto Readout = 0; Readout < NumReadouts; Readout++) {
    auto ReadoutData = (ESSReadout::VMM3Parser::VMM3Data *)DP;

    ReadoutData->DataLength = sizeof(ESSReadout::VMM3Parser::VMM3Data);
    assert(ReadoutData->DataLength == 20);

    ReadoutData->TimeHigh = TimeHigh;
    ReadoutData->TimeLow = TimeLow;
    ReadoutData->OTADC = 1000;


    if ((Readout % 16) == 0){
      XGlobal = Fuzzer.random8() * 12 / 255;
    } 
    YGlobal = 12 * abs(XChannel-2) + 140 * Readout % 16;


    ReadoutData->RingId = 5
    ReadoutData->FENId = 1 + (Readout % 2);

    // Wire
    if ((Readout % 2) == 0) {
      if Global < 2 {
        VMM = 0;
        Channel = (XGlobal * 16) + 32 + Fuzzer.random8() * 16 / 255;
      }
      else{
        VMM = 1;
        Channel = (XGlobal - 2) * 16 + Fuzzer.random8() * 16 / 255;
      }
      ReadoutData->VMM = VMM
      ReadoutData->Channel = Channel;
    }
    // Grid
    else {
      
      ReadoutData->Channel = 0;
    }

    DP += VMM3DataSize;
    if ((Readout % 3) == 0) {
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
