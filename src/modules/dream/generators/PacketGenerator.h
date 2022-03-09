// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief packet generator for dream simulated detector data
///
/// Created packets based on the ESS readout data format.
/// The common header is initialised with the data type specified in the
/// ICD for  ESS readouts.
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/readout/ess/Parser.h>
#include <string.h>


class PacketGenerator {
public:

  enum PktInit {ClearPacket, IncSeqNum};

  /// \brief prepare to generate ESS
  PacketGenerator(uint8_t Type, uint16_t ReadoutSize) :
    DataSize(ReadoutSize), ReadoutType(Type) {
    newPacket(ClearPacket);
  }

  //
  void newPacket(PktInit method) {
    php = (struct ESSReadout::Parser::PacketHeaderV0 *)buffer;

    // Either clear the buffer and reinitialze or just increment SeqNum
    if (method == IncSeqNum) {
      php->SeqNum = SeqNum++;
    } else {
      memset(buffer, 0, MaxBytes);
      php->CookieAndType = ReadoutType << 24;
      php->CookieAndType += 0x535345;
      php->SeqNum = SeqNum;
      Readouts = 0;
      BufferSize = 0;
      setLength(BufferSize);
    }
  }

  /// Add a data segment with one readout (Data Header + Data)
  void addReadout(void * readout, uint8_t Ring, uint8_t FEN)  {
    // size of 'blue' and 'white' field (readout ICD)
    uint16_t DataBlockSize = DataHeaderSize + DataSize;
    int offset = HeaderSize + Readouts * (DataBlockSize);

    struct ESSReadout::Parser::DataHeader datahdr;
    datahdr.RingId = Ring;
    datahdr.FENId = FEN;
    datahdr.DataLength = DataBlockSize;

    memcpy(buffer + offset, &datahdr, DataHeaderSize);
    memcpy(buffer + offset + DataHeaderSize, readout, DataSize);
    Readouts++;
    BufferSize = HeaderSize + (DataBlockSize) * Readouts;
    setLength(BufferSize);
  }

  uint32_t getSeqNum() { return SeqNum; }

  char * getBuffer() { return buffer; }

  uint16_t getSize() { return BufferSize; }

private:
  static const int MaxBytes{9000};
  char buffer[MaxBytes];
  struct ESSReadout::Parser::PacketHeaderV0 * php;
  uint32_t SeqNum{0};
  uint16_t Readouts{0};
  uint8_t DataHeaderSize = 4;
  uint16_t HeaderSize = sizeof(struct ESSReadout::Parser::PacketHeaderV0);
  uint16_t DataSize{0}; // set in constructor
  uint8_t ReadoutType{0}; // set in constructor
  uint16_t BufferSize{0};

  void setLength(uint16_t Length) {php->TotalLength = Length; }
};

// GCOVR_EXCL_STOP
