// Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parsing code for ADC readout.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcBufferElements.h"
#include "AdcTimeStamp.h"
#include "ChannelID.h"
#include "SamplingRun.h"
#include <cstdint>
#include <exception>
#include <functional>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <vector>

/// \brief Custom exception to handle parsing errors.
class ParserException : public std::runtime_error {
public:
  /// \brief Different types of parsing errors known by the parser.
  enum class Type {
    UNKNOWN,
    TRAILER_FEEDF00D,
    TRAILER_0x55,
    DATA_BEEFCAFE,
    DATA_LENGTH,
    DATA_ABCD,
    DATA_NO_MODULE,
    DATA_CANT_PROCESS,
    HEADER_LENGTH,
    HEADER_TYPE,
    HEADER_PROTOCOL_VERSION,
    IDLE_LENGTH,
    Count
  };

  static const char *TypeNames[] = {"UNKNOWN",
                                    "TRAILER_FEEDF00D",
                                    "TRAILER_0x55",
                                    "DATA_BEEFCAFE",
                                    "DATA_LENGTH",
                                    "DATA_ABCD",
                                    "DATA_NO_MODULE",
                                    "DATA_CANT_PROCESS",
                                    "HEADER_LENGTH",
                                    "HEADER_TYPE",
                                    "HEADER_PROTOCOL_VERSION",
                                    "IDLE_LENGTH",
                                    "Count"};

  /// \brief Sets the parsing error to the give type.
  /// \param[in] ErrorType The parser error type.
  explicit ParserException(Type ErrorType);
  
  virtual const char *what() const noexcept override;
  Type getErrorType() const;

private:
  Type ParserErrorType;
  std::string Error;

public:
  static uint64_t ErrorTypeCount[static_cast<int>(Type::Count)];
};

class ModuleProcessingException : public std::runtime_error {
public:
  explicit ModuleProcessingException(SamplingRun *Data)
      : std::runtime_error("Unable to processe data module"),
        UnproccesedData(Data) {}
  SamplingRun *UnproccesedData;
};

/// \brief Different types of data packets form teh ADC hardware.
enum class PacketType : std::uint8_t { Idle = 0x22, Data = 0x11, Unknown };

/// \brief Parsed data containing 0 or more data modules form sampling runs.
struct PacketInfo {
  std::uint16_t ReadoutCount;
  PacketType Type = PacketType::Unknown;
};

/// \brief Returned by the header parser.
struct HeaderInfo {
  PacketType Type = PacketType::Unknown;
  TimeStamp ReferenceTimestamp;
  std::uint16_t ReadoutCount{0};
  std::int32_t DataStart{0};
};

/// \brief Returned by the trailer parser.
struct TrailerInfo {
  std::uint32_t FillerBytes = 0;
};

/// \brief Returned by the idle packet parser.
struct IdleInfo {
  RawTimeStamp TimeStamp;
  std::int32_t FillerStart = 0;
};

enum class Protocol : std::uint8_t {
  VER_0 = 0x11,
  VER_0_1 = 0x11,
  VER_0_2 = 0x22,
  VER_1 = 0x01,
};

enum class Clock : std::uint8_t {
  Clk_Ext = 0x01,
  Clk_45MHz = 0x00,
};

#pragma pack(push, 2)
/// \brief Used by the header parser to map types to the binary data.
struct PacketHeader {
  Protocol Version;
  PacketType Type;
  std::uint16_t ReadoutLength;
  std::uint16_t ReadoutCount;
  Clock ClockMode;
  std::uint8_t OversamplingFactor;
  RawTimeStamp ReferenceTimeStamp;
  void fixEndian() {
    ReadoutLength = ntohs(ReadoutLength);
    ReadoutCount = ntohs(ReadoutCount);
    ReferenceTimeStamp.fixEndian();
  }
} __attribute__((packed));

/// \brief Used by the data parser to map types to the binary data.
struct DataHeader {
  std::uint16_t MagicValue;
  std::uint16_t Length;
  std::uint16_t Channel;
  std::uint16_t Oversampling;
  RawTimeStamp TimeStamp;
  void fixEndian() {
    MagicValue = ntohs(MagicValue);
    Length = ntohs(Length);
    Channel = ntohs(Channel);
    Oversampling = ntohs(Oversampling);
    TimeStamp.fixEndian();
  }
} __attribute__((packed));

/// \brief Used by the idle parser to map types to the binary data.
struct IdleHeader {
  RawTimeStamp TimeStamp;
  void fixEndian() { TimeStamp.fixEndian(); }
} __attribute__((packed));
#pragma pack(pop)

class PacketParser {
public:
  /// \brief Constructor, one instance should only handle data from a single
  /// data source (ADC box).
  /// \param[in] PushDataModuleToQueue Function for submitting the result of a
  /// parsed packet to a handler which does further processing.
  /// \param[in] PullDataModuleFromQueue Function for getting an empty module for
  /// storing processed data into. 
  /// \param[in] SourceID An integer used to
  /// identify the data source. This value is passed on together with the parsed
  /// data.
  PacketParser(std::function<bool(SamplingRun *)> PushDataModuleToQueue,
               std::function<SamplingRun *(ChannelID Identifier)>
                   PullDataModuleFromQueue,
               std::uint16_t SourceID);
  /// \brief Parses a packet of binary data.
  /// \param[in] Packet Raw data, straight from the socket.
  /// \return Some general information about the packet.
  /// \throw ParserException See exception type for possible parsing failures.
  PacketInfo parsePacket(const InData &Packet);

protected:
  /// \brief Parses the payload of a packet. Called by parsePacket().
  /// \param[in] Packet Raw data buffer.
  /// \param[in] StartByte The byte on which the payload starts.
  /// \param[in] ReferenceTimestamp A reference timestamp as supplied with by
  /// the readout system.
  /// \return The start of the filler/trailer in the array.
  /// \throw ParserException See exception type for possible parsing failures.
  size_t parseData(const InData &Packet, std::uint32_t StartByte,
                   TimeStamp const &ReferenceTimestamp);

private:
  std::function<bool(SamplingRun *)> PushDataModuleToQueue;
  std::function<SamplingRun *(ChannelID Identifier)> PullDataModuleFromQueue;
  std::uint16_t Source;
};

/// \brief Parses the header of a packet. Called by parsePacket().
/// \param[in] Packet Raw data buffer.
/// \return Data extracted from the header and an integer indicating the start
/// of the payload.
/// \throw ParserException See exception type for possible parsing failures.
HeaderInfo parseHeader(const InData &Packet);

/// \brief Checks the conents and the size of the packet filler as well as the
/// trailer. Called by parsePacket().
/// \param[in] Packet Raw data buffer.
/// \param[in] StartByte The byte at which the filler/trailer starts.
/// \return The number of bytes in the filler.
/// \throw ParserException See exception type for possible parsing failures.
TrailerInfo parseTrailer(const InData &Packet, std::uint32_t StartByte);

/// \brief Parses the idle packet payload. Called by parsePacket().
/// \param[in] Packet Raw data buffer.
/// \param[in] StartByte First byte of the idle packet payload.
/// \return Idle packet timestamp.
/// \throw ParserException See exception type for possible parsing failures.
IdleInfo parseIdle(const InData &Packet, std::uint32_t StartByte);

struct ConfigInfo {
  enum class Version { VER_0, VER_1 } ProtocolVersion;
  TimeStamp BaseTime;
};

/// \brief Extract information from the header of a packet for the purpose of
/// setting up the parsing of subsequent packets.
/// \param[in] Packet Raw data buffer.
/// \return Packet information, i.e. protocol version and time stamp.
/// \throw ParserException See exceptions type for possible parsing failures.
ConfigInfo parseHeaderForConfigInfo(const InData &Packet);
