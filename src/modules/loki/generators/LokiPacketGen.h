// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief packet generator for loki detector data
///
/// Creates a buffer ready for transmission.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <loki/readout/DataParser.h>
#include <readout/ReadoutParser.h>
#include <string.h>

class LokiPacketGen {
public:

  LokiPacketGen() {
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
    php->CookieAndType = ReadoutParser::Loki4Amp << 24;
    php->CookieAndType += 0x535345;
    php->SeqNum = SeqNum++;
    Readouts = 0;
  }

  void setLength(uint16_t Length) {php->TotalLength = Length; }

  /// Add a data segment with one readout (Data Header + Data)
  uint16_t addReadout(struct Loki::DataParser::LokiReadout & rdout, uint8_t Ring, uint8_t FEN)  {
    int offset = HeaderSize + Readouts * (DataSize + DataHeaderSize);
    uint32_t datahdr = 0x00180000 + (FEN << 8) + Ring;
    memcpy(buffer + offset, &datahdr, DataHeaderSize);
    memcpy(buffer + offset + DataHeaderSize, &rdout, DataSize);
    Readouts++;
    return HeaderSize + (DataSize + 4) * Readouts;
  }

  uint32_t getSeqNum() { return SeqNum; }

  char * getBuffer() { return buffer; }

private:
  static const int MaxBytes{9000};
  char buffer[MaxBytes];
  struct ReadoutParser::PacketHeaderV0 * php;
  uint32_t SeqNum{0};
  uint16_t Readouts{0};
  uint8_t DataHeaderSize = 4;
  uint16_t HeaderSize = sizeof(struct ReadoutParser::PacketHeaderV0);
  uint16_t DataSize = (uint16_t)sizeof(struct Loki::DataParser::LokiReadout);
};
