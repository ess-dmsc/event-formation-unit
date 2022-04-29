// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial VMM3 readouts
// based on VMM3 Readout ICD document
// https://project.esss.dk/owncloud/index.php/f/14670413
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <common/debug/Trace.h>
#include <math.h>
#include <time.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <stdexcept>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB



ReadoutGeneratorBase::ReadoutGeneratorBase(uint8_t *BufferPtr, uint16_t MaxPayloadSize,
  uint32_t InitialSeqNum, GeneratorSettings& Settings)
  : Settings(Settings)
  , Buffer(BufferPtr)
  , BufferSize(MaxPayloadSize)
  , SeqNum(InitialSeqNum)
  {}


uint16_t ReadoutGeneratorBase::makePacket() {
  generateHeader();
  generateData();
  finishPacket();
  return DataSize;
}

void ReadoutGeneratorBase::generateHeader() {

  DataSize = HeaderSize + Settings.NumReadouts * ReadoutDataSize;
  if (DataSize >= BufferSize) {
    throw std::runtime_error("Too many readouts for buffer size");
  }

  memset(Buffer, 0, BufferSize);
  auto Header = (ESSReadout::Parser::PacketHeaderV0 *)Buffer;

  Header->CookieAndType = (Settings.Type << 24) + 0x535345;
  Header->Padding0 = 0;
  Header->Version = 0;
  // Header->OutputQueue = 0x00;

  Header->TotalLength = DataSize;
  Header->SeqNum = SeqNum;
  Header->PulseHigh = TimeHigh;
  Header->PulseLow = TimeLow;
  Header->PrevPulseHigh = TimeHigh;
  Header->PrevPulseLow = TimeLow;

  XTRACE(DATA, DEB, "new packet header, time high %u, time low %u", TimeHigh, TimeLow);
}

void ReadoutGeneratorBase::finishPacket() {
  SeqNum++; // ready for next packet

  // if doing fuzzing, fuzz up to one field in header & up to 20 fields in data
  if (Settings.Randomise) {
    Fuzzer.fuzz8Bits(Buffer, HeaderSize, 1);
    Fuzzer.fuzz8Bits(Buffer + HeaderSize, DataSize - HeaderSize, 20);
  }
}

// GCOVR_EXCL_STOP
