/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Parsing code for ADC readout.
 */

#pragma once

#include "AdcBufferElements.h"
#include "AdcTimeStamp.h"
#include <exception>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <vector>

/// @brief Custom exception to handle parsing errors.
class ParserException : public std::runtime_error {
public:
  /// @brief Different types of parsing errors known by the parser.
  enum class Type {
    UNKNOWN,
    TRAILER_FEEDF00D,
    TRAILER_0x55,
    DATA_BEEFCAFE,
    DATA_LENGTH,
    DATA_ABCD,
    STREAM_HEADER,
    STREAM_ABCD,
    STREAM_TYPE,
    STREAM_OVERSAMPLING,
    STREAM_CHANNELS_MASK,
    STREAM_SIZE,
    STREAM_BEEFCAFE,
    HEADER_LENGTH,
    HEADER_TYPE,
    IDLE_LENGTH,
  };
  /// @brief Sets the (parsing) error type to Type::UNKNOWN.
  /// @param[ErrorStr] The string describing the exception.
  ParserException(std::string const &ErrorStr);
  /// @brief Sets the parsing error to the give type.
  /// @param[ErrorType] The parser error type.
  ParserException(Type ErrorType);
  virtual const char *what() const noexcept override;
  Type getErrorType() const;

private:
  Type ParserErrorType;
  std::string Error;
};

/// @brief Data stored in this struct represents a (properly parsed) sampling run.
struct DataModule {
  DataModule() = default;
  RawTimeStamp TimeStamp;
  std::uint16_t Channel;
  std::uint16_t OversamplingFactor{1};
  std::vector<std::uint16_t> Data;
};

/// @brief Output from the payload parser.
struct AdcData {
  std::vector<DataModule> Modules;
  std::int32_t FillerStart = 0;
};

/// @brief Different types of data packets form teh ADC hardware.
enum class PacketType { Idle, Data, Stream, Unknown };

/// @brief Parsed data containing 0 or more data modules form sampling runs.
struct PacketData {
  std::uint16_t GlobalCount;
  std::uint16_t ReadoutCount;
  PacketType Type = PacketType::Unknown;
  std::vector<DataModule> Modules;
  RawTimeStamp IdleTimeStamp;
};

/// @brief Returned by the header parser.
struct HeaderInfo {
  PacketType Type = PacketType::Unknown;
  std::uint16_t GlobalCount;
  std::uint16_t ReadoutCount;
  std::int32_t DataStart = 0;
  std::uint16_t TypeValue; // Used only by streaming data
};

/// @brief Returned by the trailer parser.
struct TrailerInfo {
  std::uint32_t FillerBytes = 0;
};

/// @brief Returned by the idle packet parser.
struct IdleInfo {
  RawTimeStamp TimeStamp;
  std::int32_t FillerStart = 0;
};


struct StreamSetting {
  std::vector<int> ChannelsActive;
  int OversamplingFactor{1};
};

#pragma pack(push, 2)
struct PacketHeader {
  std::uint16_t GlobalCount;
  std::uint16_t PacketType;
  std::uint16_t ReadoutLength;
  std::uint16_t ReadoutCount;
  std::uint16_t Reserved;
  void fixEndian() {
    GlobalCount = ntohs(GlobalCount);
    PacketType = ntohs(PacketType);
    ReadoutLength = ntohs(ReadoutLength);
    ReadoutCount = ntohs(ReadoutCount);
  }
} __attribute__((packed));

struct DataHeader {
  std::uint16_t MagicValue;
  std::uint16_t Length;
  std::uint16_t Channel;
  std::uint16_t Fragment;
  RawTimeStamp TimeStamp;
  void fixEndian() {
    MagicValue = ntohs(MagicValue);
    Length = ntohs(Length);
    Channel = ntohs(Channel);
    Fragment = ntohs(Fragment);
    TimeStamp.fixEndian();
  }
} __attribute__((packed));

struct StreamHeader {
  std::uint16_t MagicValue;
  std::uint16_t Length;
  RawTimeStamp TimeStamp;
  void fixEndian() {
    MagicValue = ntohs(MagicValue);
    Length = ntohs(Length);
    TimeStamp.fixEndian();
  }
} __attribute__((packed));

struct IdleHeader {
  RawTimeStamp TimeStamp;
  void fixEndian() { TimeStamp.fixEndian(); }
} __attribute__((packed));
#pragma pack(pop)

PacketData parsePacket(const InData &Packet);
AdcData parseData(const InData &Packet, std::uint32_t StartByte);
AdcData parseStreamData(const InData &Packet, std::uint32_t StartByte,
                        std::uint16_t TypeValue);
HeaderInfo parseHeader(const InData &Packet);
TrailerInfo parseTrailer(const InData &Packet, std::uint32_t StartByte);
IdleInfo parseIdle(const InData &Packet, std::uint32_t StartByte);
StreamSetting parseStreamSettings(const std::uint16_t &SettingsRaw);
