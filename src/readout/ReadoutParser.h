/* Copyright (C) 2016-2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS Readout System Data definitions and parsing functions
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>

const uint8_t MaxOutputQueues{64};

#define READOUT_EXTRA_PADDING
#ifdef READOUT_EXTRA_PADDING
  #define PAD_SIZE 2
  #define EXTRA_PADDING 0x00, 0x00,
#else
  #define PAD_SIZE 0
  #define EXTRA_PADDING
#endif

class ReadoutParser {
public:
  enum error { OK = 0, EBUFFER, ESIZE, EHEADER };
  enum DetectorType { Loki4Amp = 0x30 };

  uint64_t NextSeqNum[MaxOutputQueues];

  // Header common to all ESS readout data
  struct PacketHeaderV0 {
  #ifdef READOUT_EXTRA_PADDING
    uint16_t Padding;
  #endif
    uint32_t CookieVersion;
    uint8_t TypeSubType;
    uint8_t OutputQueue;
    uint16_t TotalLength;
    uint32_t PulseHigh;
    uint32_t PulseLow;
    uint32_t PrevPulseHigh;
    uint32_t PrevPulseLow;
    uint32_t SeqNum;
  } __attribute__((packed));

  // Holds data relevant for processing of the current packet
  struct {
    PacketHeaderV0 * HeaderPtr;
    uint16_t DataLength;
    char * DataPtr;
  } Packet;

  // Header for each data block
  /// \todo change order to uint16, uint8, uint8
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
    int64_t ErrorVersion{0};
    int64_t ErrorTypeSubType{0};
    int64_t ErrorOutputQueue{0};
    int64_t ErrorSeqNum{0};
  } Stats;
};

static_assert(sizeof(ReadoutParser::PacketHeaderV0) == (28 + PAD_SIZE),
              "Wrong header size (update assert or check packing)");
