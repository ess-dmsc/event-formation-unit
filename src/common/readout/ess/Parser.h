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
#include <common/readout/ess/ESSTime.h>

namespace ESSReadout {

struct ESSHeaderStats {
  int64_t ErrorBuffer{0};
  int64_t ErrorSize{0};
  int64_t ErrorVersion{0};
  int64_t ErrorCookie{0};
  int64_t ErrorPad{0};
  int64_t ErrorOutputQueue{0};
  int64_t ErrorTypeSubType{0};
  int64_t ErrorSeqNum{0};
  int64_t ErrorTimeHigh{0};
  int64_t ErrorTimeFrac{0};
  int64_t HeartBeats{0};
};

const uint32_t MaxFracTimeCount{88'052'499};
const uint8_t MaxOutputQueues{24};
const unsigned int MaxUdpDataSize{8972};
const unsigned int MinDataSize{5}; // just pad, cookie and version

class Parser {
public:
  enum error { OK = 0, EBUFFER, ESIZE, EHEADER };
  enum DetectorType {
    Reserved = 0x00,
    TTLMonitor = 0x10,
    LOKI = 0x30,
    BIFROST = 0x34,
    MIRACLES = 0x38,
    CSPEC = 0x40,
    NMX = 0x44,
    FREIA = 0x48,
    DREAM = 0x60
  };

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

  static_assert(sizeof(Parser::PacketHeaderV0) == (30),
                "Wrong header size (update assert or check packing)");

  // Holds data relevant for processing of the current packet
  struct PacketDataV0 {
    PacketHeaderV0 *HeaderPtr{nullptr};
    uint16_t DataLength{0};
    char *DataPtr{nullptr};
    ESSTime Time;
  } Packet;

  // Header for each data block
  struct DataHeader {
    uint8_t RingId;
    uint8_t FENId;
    uint16_t DataLength;
  } __attribute__((packed));

  //
  Parser();

  //
  void setMaxPulseTimeDiff(uint32_t MaxTimeDiff) {
    MaxPulseTimeDiffNS = MaxTimeDiff;
  }

  /// \brief validate a readout buffer
  /// \param[in] Buffer pointer to data
  /// \param[in] Size length of buffer in bytes
  /// \param[in] Type expected detector type
  /// \return on success return 0, else < 0
  int validate(const char *Buffer, uint32_t Size, uint8_t Type);

  // Counters(for Grafana)
  struct ESSHeaderStats Stats;
  // Maximum allowed separation between PulseTime and PrevPulseTime
  ///\todo 6289464 is (maybe) 14Hz in ticks of ESS clock, or?
  /// 71428571 in ns, but for now we set max pt to 0 and require
  /// setting this in the config file.
  uint32_t MaxPulseTimeDiffNS{0};
};
} // namespace ESSReadout
