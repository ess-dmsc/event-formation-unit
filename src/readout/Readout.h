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

class Readout {
public:
  enum error { OK = 0, EBUFFER, ESIZE, EHEADER };

  // Header common to all ESS readout data
  struct PacketHeaderV0 {
    uint32_t CookieVersion;
    uint8_t TypeSubType;
    uint8_t OutputQueue;
    uint16_t TotalLength;
    uint32_t SeqNum;
    uint32_t PulseHigh;
    uint32_t PulseLow;
    uint32_t PrevPulseHigh;
    uint32_t PrevPulseLow;
  } Header __attribute__((packed));

  // Header for eack data block
  struct DataHeader {
    uint8_t RingId;
    uint8_t FENId;
    uint16_t DataLength;
  } __attribute__((packed));

  /// \brief validate a readout buffer
  /// \param[in] buffer pointer to data
  /// \param[in] size length of buffer in bytes
  /// \return on success return 0, else -1
  int validate(const char *Buffer, uint32_t Size, uint8_t Type);

  // For Grafana eventually
  struct {
    int64_t ErrorBuffer{0};
    int64_t ErrorSize{0};
    int64_t ErrorVersion{0};
    int64_t ErrorTypeSubType{0};
  } Stats;
};

static_assert(sizeof(Readout::PacketHeaderV0) == 28,
              "Wrong header size (update assert or check packing)");
