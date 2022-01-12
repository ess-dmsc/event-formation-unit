// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
// based on VMM3 Readout ICD document https://project.esss.dk/owncloud/index.php/f/14670413
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <math.h>
#include <time.h>
#include <common/readout/vmm3/generators/ReadoutGeneratorBase.h>
#include <stdexcept>



ReadoutGeneratorBase::ReadoutGeneratorBase(uint8_t *BufferPtr, uint16_t MaxPayloadSize,
  uint32_t InitialSeqNum, bool Randomise)
  : Buffer(BufferPtr)
  , BufferSize(MaxPayloadSize)
  , SeqNum(InitialSeqNum)
  , Random(Randomise) { }


uint16_t ReadoutGeneratorBase::makePacket(
  uint8_t Type, uint16_t NumReadouts,
  uint32_t TicksBtwReadouts, uint32_t TicksBtwEvents) {
  TimeBtwReadout = TicksBtwReadouts;
  TimeBtwEvents = TicksBtwEvents;
  generateHeader(Type, NumReadouts);
  generateData(NumReadouts);
  finishPacket();
  return DataSize;
}


void ReadoutGeneratorBase::generateHeader(uint8_t Type, uint16_t NumReadouts) {

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


void ReadoutGeneratorBase::finishPacket() {
  SeqNum++; // ready for next packet

  // if doing fuzzing, fuzz up to one field in header & up to 20 fields in data
  if (Random) {
    Fuzzer.fuzz8Bits(Buffer, HeaderSize, 1);
    Fuzzer.fuzz8Bits(Buffer + HeaderSize, DataSize - HeaderSize, 20);
  }
}


// GCOVR_EXCL_STOP
