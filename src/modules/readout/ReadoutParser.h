// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS Readout System Data definitions and parsing functions
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>

const uint8_t MaxOutputQueues{24};

class ReadoutParser {
public:
  enum error { OK = 0, EBUFFER, ESIZE, EHEADER };
  enum DetectorType { Reserved = 0x00, Loki4Amp = 0x30, DREAM = 0x60};

  uint64_t NextSeqNum[MaxOutputQueues];

  // Header common to all ESS readout data
  // Reviewed ICD (version 2) packet header version 0
  // ownCloud: https://project.esss.dk/owncloud/index.php/s/DWNer23727TiI1x
  struct PacketHeaderV0 {
    uint8_t Padding0;
    uint8_t Version;
    uint32_t CookieAndType;
    uint16_t TotalLength;
    uint8_t OutputQueue;
    uint8_t TimeSource;
    uint32_t PulseHigh;
    uint32_t PulseLow;
    uint32_t PrevPulseHigh;
    uint32_t PrevPulseLow;
    uint32_t SeqNum;
  } __attribute__((packed));

  static_assert(sizeof(ReadoutParser::PacketHeaderV0) == (30),
                "Wrong header size (update assert or check packing)");


  // Holds data relevant for processing of the current packet
  struct {
    PacketHeaderV0 * HeaderPtr;
    uint16_t DataLength;
    char * DataPtr;
  } Packet;

  // Header for each data block
  struct DataHeader {
    uint8_t RingId;
    uint8_t FENId;
    uint16_t DataLength;
  } __attribute__((packed));


  ReadoutParser();

  /// \brief validate a readout buffer
  /// \param[in] Buffer pointer to data
  /// \param[in] Size length of buffer in bytes
  /// \param[in] Type expected detector type
  /// \return on success return 0, else < 0
  int validate(const char *Buffer, uint32_t Size, uint8_t Type);

  // Counters(for Grafana)
  struct {
    int64_t ErrorBuffer{0};
    int64_t ErrorSize{0};
    int64_t ErrorPad{0};
    int64_t ErrorVersion{0};
    int64_t ErrorCookie{0};
    int64_t ErrorTypeSubType{0};
    int64_t ErrorOutputQueue{0};
    int64_t ErrorSeqNum{0};
  } Stats;
};
