// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief packet generator for dream simulated detector data
///
/// Creates a buffer ready for transmission.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <readout/ReadoutParser.h>
#include <string.h>

// GCOVR_EXCL_START

class PacketGenerator {
public:

  PacketGenerator(uint8_t type, uint16_t readout_size) :
    DataSize(readout_size), ReadoutType(type) {
    newPacket();
  }

  // Keep data (for retransmissions - speed)
  void nextSeqNo() {
    php = (struct ReadoutParser::PacketHeaderV0 *)buffer;
    php->SeqNum = SeqNum++;
  }

  //
  void newPacket() {
    memset(buffer, 0, MaxBytes);

    php = (struct ReadoutParser::PacketHeaderV0 *)buffer;
    php->CookieAndType = ReadoutType << 24;
    php->CookieAndType += 0x535345;
    php->SeqNum = SeqNum;
    Readouts = 0;
    BufferSize = 0;
    setLength(BufferSize);
  }

  /// Add a data segment with one readout (Data Header + Data)
  void addReadout(void * readout, uint8_t Ring, uint8_t FEN)  {
    int offset = HeaderSize + Readouts * (DataSize + DataHeaderSize);
    uint32_t datahdr = 0x00180000 + (FEN << 8) + Ring;
    memcpy(buffer + offset, &datahdr, DataHeaderSize);
    memcpy(buffer + offset + DataHeaderSize, readout, DataSize);
    Readouts++;
    BufferSize = HeaderSize + (DataSize + DataHeaderSize) * Readouts;
    setLength(BufferSize);
  }

  uint32_t getSeqNum() { return SeqNum; }

  char * getBuffer() { return buffer; }

  uint16_t getSize() { return BufferSize; }

private:
  static const int MaxBytes{9000};
  char buffer[MaxBytes];
  struct ReadoutParser::PacketHeaderV0 * php;
  uint32_t SeqNum{0};
  uint16_t Readouts{0};
  uint8_t DataHeaderSize = 4;
  uint16_t HeaderSize = sizeof(struct ReadoutParser::PacketHeaderV0);
  uint16_t DataSize{0}; // set in constructor
  uint8_t ReadoutType{0}; // set in constructor
  uint16_t BufferSize{0};

  void setLength(uint16_t Length) {php->TotalLength = Length; }
};

// GCOVR_EXCL_STOP
