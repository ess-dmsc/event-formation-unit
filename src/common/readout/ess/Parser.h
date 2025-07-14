// Copyright (C) 2017 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS Readout System Data definitions and parsing functions. For
/// further details, see the readout ICD
///
///   https://project.esss.dk/nextcloud/index.php/apps/files/files/16376550?dir=/DM/detectors/02%20instruments/01%20common/02%20Readout/02%20ICD
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/time/ESSTime.h>
#include <common/types/DetectorType.h>

#include <cinttypes>
#include <cstddef>
#include <cstdint>

namespace ESSReadout {

using namespace esstime;

const uint32_t MaxFracTimeCount{88'052'499};
const uint8_t MaxOutputQueues{12};
const unsigned int MaxUdpDataSize{8972};
const unsigned int MinDataSize{5}; // just pad, cookie and version

class Parser {

public:
  // Static const strings for stat names
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_HEADER = "parser.essheader.errors.header";
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_BUFFER = "parser.essheader.errors.buffer";
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_COOKIE = "parser.essheader.errors.cookie";
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_PAD = "parser.essheader.errors.pad";
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_SIZE = "parser.essheader.errors.size";
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_VERSION = "parser.essheader.errors.version";
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_OUTPUT_QUEUE = "parser.essheader.errors.output_queue";
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_TYPE = "parser.essheader.errors.type";
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_SEQNO = "parser.essheader.errors.seqno";
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_TIMEHIGH = "parser.essheader.errors.timehigh";
  static inline const std::string METRIC_PARSER_ESSHEADER_ERRORS_TIMEFRAC = "parser.essheader.errors.timefrac";
  static inline const std::string METRIC_PARSER_ESSHEADER_HEARTBEATS = "parser.essheader.heartbeats";
  static inline const std::string METRIC_PARSER_ESSHEADER_VERSION_V0 = "parser.essheader.version.v0";
  static inline const std::string METRIC_PARSER_ESSHEADER_VERSION_V1 = "parser.essheader.version.v1";
  static inline const std::string METRIC_PARSER_ESSHEADER_OQ_PACKETS = "parser.essheader.OQ.{:02}.packets";
  
  static inline const std::string METRIC_EVENTS_TIMESTAMP_TOF_COUNT = "events.timestamp.tof.count";
  static inline const std::string METRIC_EVENTS_TIMESTAMP_TOF_NEGATIVE = "events.timestamp.tof.negative";
  static inline const std::string METRIC_EVENTS_TIMESTAMP_TOF_HIGH = "events.timestamp.tof.high";
  static inline const std::string METRIC_EVENTS_TIMESTAMP_PREVTOF_COUNT = "events.timestamp.prevtof.count";
  static inline const std::string METRIC_EVENTS_TIMESTAMP_PREVTOF_NEGATIVE = "events.timestamp.prevtof.negative";
  static inline const std::string METRIC_EVENTS_TIMESTAMP_PREVTOF_HIGH = "events.timestamp.prevtof.high";

private:
  struct ESSHeaderStats {
    int64_t ErrorHeader{0};
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
    int64_t Version0Header{0};
    int64_t Version1Header{0};
    int64_t HeartBeats{0};
    int64_t OQRxPackets[MaxOutputQueues]{0};
  } ESSHeaderStats;

public:
  /// \brief Constructor of the parser
  /// \param reference to the statistics object to register internal counters
  Parser(Statistics &);

  enum HeaderVersion { V0 = 0x00, V1 = 0x01 };
  enum error { OK = 0, EBUFFER, ESIZE, EHEADER };
  uint64_t NextSeqNum[MaxOutputQueues];

  // Header common to all ESS readout data
  // Reviewed ICD (version 2) packet header version 0
  // ownCloud: https://project.esss.dk/nextcloud/index.php/s/DWNer23727TiI1x
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

  // Header common to all ESS readout data
  // Reviewed ICD (version 2) packet header version 0
  // ownCloud: https://project.esss.dk/nextcloud/index.php/s/DWNer23727TiI1x
  struct PacketHeaderV1 {
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
    uint16_t CMACPadd;
  } __attribute__((packed));

  class PacketHeader {
    union {
      PacketHeaderV0 *headerVersion0;
      PacketHeaderV1 *headerVersion1;
    };

    size_t size{0};

  public:
    PacketHeader() = default;

    PacketHeader(PacketHeaderV0 *newHeader)
        : headerVersion0(newHeader), size(sizeof(*newHeader)){};
    PacketHeader(PacketHeaderV1 *newHeader)
        : headerVersion1(newHeader), size(sizeof(*newHeader)){};

    // Getters for PacketHeaderV0 members
    uint8_t getVersion() const { return headerVersion0->Version; }
    uint32_t getCookieAndType() const { return headerVersion0->CookieAndType; }
    uint16_t getTotalLength() const { return headerVersion0->TotalLength; }
    uint8_t getOutputQueue() const { return headerVersion0->OutputQueue; }
    uint8_t getTimeSource() const { return headerVersion0->TimeSource; }
    uint32_t getPulseHigh() const { return headerVersion0->PulseHigh; }
    uint32_t getPulseLow() const { return headerVersion0->PulseLow; }
    uint32_t getPrevPulseHigh() const { return headerVersion0->PrevPulseHigh; }
    uint32_t getPrevPulseLow() const { return headerVersion0->PrevPulseLow; }
    uint32_t getSeqNum() const { return headerVersion0->SeqNum; }
    size_t getSize() const { return size; }
  };

  static_assert(sizeof(Parser::PacketHeaderV0) == (30),
                "Wrong header size (update assert or check packing)");

  static_assert(sizeof(Parser::PacketHeaderV1) == (32),
                "Wrong header size (update assert or check packing)");

  // Holds data relevant for processing of the current packet
  struct PacketDataV0 {
    PacketHeader HeaderPtr;
    uint16_t DataLength{0};
    char *DataPtr{nullptr};
    ESSReferenceTime Time;
  } Packet;

  // Header for each data block
  struct DataHeader {
    uint8_t FiberId;
    uint8_t FENId;
    uint16_t DataLength;
  } __attribute__((packed));

  //
  void setMaxPulseTimeDiff(uint32_t MaxTimeDiff) {
    MaxPulseTimeDiffNS = TimeDurationNano(MaxTimeDiff);
  }

  /// \brief validate a readout buffer
  /// \param[in] Buffer pointer to data
  /// \param[in] Size length of buffer in bytes
  /// \param[in] Type expected detector type
  /// \return on success return 0, else < 0
  int validate(const char *Buffer, uint32_t Size, uint8_t Type);

  // Maximum allowed separation between PulseTime and PrevPulseTime
  ///\todo 6289464 is (maybe) 14Hz in ticks of ESS clock, or?
  /// 71428571 in ns, but for now we set max pt to 0 and require
  /// setting this in the config file.
  TimeDurationNano MaxPulseTimeDiffNS{0};
};
} // namespace ESSReadout
