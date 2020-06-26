/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief ADC data parsing code.
 */

#include "AdcParse.h"
#include <arpa/inet.h>
#include <bitset>
#include <cstring>
#include <map>
#include <netinet/in.h>

static const std::uint32_t PACKET_TRAILER{0xFEEDF00D};
static const std::uint16_t MODULE_HEADER{0xABCD};
static const std::uint32_t MODULE_TRAILER{0xBEEFCAFE};
static const std::uint8_t FILLER_BYTE{0x55};
static const std::uint16_t TWO_FILLER_BYTES{0x5555};
static const std::uint32_t PACKET_LENGTH_OFFSET{8u};

uint64_t ParserException::ErrorTypeCount[static_cast<int>(Type::Count)] = {0};

ParserException::ParserException(Type ErrorType)
    : std::runtime_error("Parser error of type " +
                         std::to_string(static_cast<int>(ErrorType))),
      ParserErrorType(ErrorType) {
  if (ErrorType < Type::Count) {
    ErrorTypeCount[static_cast<int>(ErrorType)]++;
  }
}

ParserException::Type ParserException::getErrorType() const {
  return ParserErrorType;
}

const char *ParserException::what() const noexcept {
  static std::map<ParserException::Type, std::string> ErrorTypeStrings = {
      {ParserException::Type::TRAILER_FEEDF00D,
       "Magic bytes (0xFEEDF00D) not found at the end of the packet."},
      {ParserException::Type::TRAILER_0x55,
       "Filler byte was not of value 0x55."},
      {ParserException::Type::DATA_BEEFCAFE,
       "Magic bytes (0xBEEFCAFE) was not at the end of the data module."},
      {ParserException::Type::DATA_LENGTH,
       "Packet size to short to hold expected number of samples."},
      {ParserException::Type::DATA_NO_MODULE,
       "Did not get data module instance to store de-serialised samples."},
      {ParserException::Type::DATA_CANT_PROCESS, "Unable to process samples."},
      {ParserException::Type::DATA_ABCD,
       "Data module did not start with magic bytes (0xABCD)."},
      {ParserException::Type::HEADER_LENGTH,
       "Packet size was to short to hold header and expected payload."},
      {ParserException::Type::HEADER_TYPE, "Indicated packet type is unknown."},
      {ParserException::Type::HEADER_PROTOCOL_VERSION,
       "Unknown protocol version encountered in header."},
      {ParserException::Type::IDLE_LENGTH,
       "Packet size was to short to hold idle time stamp."},
  };
  if (ParserException::Type::UNKNOWN == ParserErrorType) {
    return Error.c_str();
  }
  if (ErrorTypeStrings.find(ParserErrorType) == ErrorTypeStrings.end()) {
    return "ParserException error string not defined for exceptions of this "
           "type.";
  }
  return ErrorTypeStrings.at(ParserErrorType).c_str();
}

PacketParser::PacketParser(
    std::function<bool(SamplingRun *)> PushDataModuleToQueue,
    std::function<SamplingRun *(ChannelID Identifier)> PullDataModuleFromQueue,
    std::uint16_t SourceID)
    : PushDataModuleToQueue(std::move(PushDataModuleToQueue)),
      PullDataModuleFromQueue(std::move(PullDataModuleFromQueue)),
      Source(SourceID) {}

PacketInfo PacketParser::parsePacket(const InData &Packet) {
  HeaderInfo Header = parseHeader(Packet);
  PacketInfo ReturnData;
  ReturnData.ReadoutCount = Header.ReadoutCount;
  if (PacketType::Data == Header.Type) {
    size_t FillerStart =
        parseData(Packet, Header.DataStart, Header.ReferenceTimestamp);
    parseTrailer(Packet, FillerStart);
    ReturnData.Type = PacketType::Data;
  } else if (PacketType::Idle == Header.Type) {
    parseIdle(Packet, Header.DataStart);
    ReturnData.Type = PacketType::Idle;
  }
  return ReturnData;
}

HeaderInfo parseHeader(const InData &Packet) {
  HeaderInfo ReturnInfo;
  if (Packet.Length < sizeof(PacketHeader)) {
    throw ParserException(ParserException::Type::HEADER_LENGTH);
  }
  auto HeaderRaw = reinterpret_cast<const PacketHeader *>(Packet.Data);
  PacketHeader Header(*HeaderRaw);
  Header.fixEndian();
  if (Header.Type != PacketType::Idle and Header.Type != PacketType::Data) {
    throw ParserException(ParserException::Type::HEADER_TYPE);
  }
  ReturnInfo.Type = Header.Type;
  ReturnInfo.DataStart = sizeof(PacketHeader);
  ReturnInfo.ReadoutCount = Header.ReadoutCount;
  ReturnInfo.ReferenceTimestamp = {Header.ReferenceTimeStamp,
                                   TimeStamp::ClockMode(Header.ClockMode)};
  if (Packet.Length != Header.ReadoutLength + PACKET_LENGTH_OFFSET) {
    throw ParserException(ParserException::Type::HEADER_LENGTH);
  }
  return ReturnInfo;
}

ConfigInfo parseHeaderForConfigInfo(const InData &Packet) {
  ConfigInfo ReturnInfo;
  if (Packet.Length < sizeof(PacketHeader)) {
    throw ParserException(ParserException::Type::HEADER_LENGTH);
  }
  auto HeaderRaw = reinterpret_cast<const PacketHeader *>(Packet.Data);
  PacketHeader Header(*HeaderRaw);
  Header.fixEndian();
  if (Header.Type != PacketType::Idle and Header.Type != PacketType::Data) {
    throw ParserException(ParserException::Type::HEADER_TYPE);
  }
  std::map<Protocol, ConfigInfo::Version> VersionMap{
      {Protocol::VER_0_1, ConfigInfo::Version::VER_0},
      {Protocol::VER_0_2, ConfigInfo::Version::VER_0},
      {Protocol::VER_1, ConfigInfo::Version::VER_1}};
  if (VersionMap.find(Header.Version) == VersionMap.end()) {
    throw ParserException(ParserException::Type::HEADER_PROTOCOL_VERSION);
  }
  ReturnInfo.ProtocolVersion = VersionMap[Header.Version];

  ReturnInfo.BaseTime = {Header.ReferenceTimeStamp,
                         TimeStamp::ClockMode(Header.ClockMode)};
  if (Packet.Length != Header.ReadoutLength + PACKET_LENGTH_OFFSET) {
    throw ParserException(ParserException::Type::HEADER_LENGTH);
  }
  return ReturnInfo;
}

size_t PacketParser::parseData(const InData &Packet, std::uint32_t StartByte,
                               TimeStamp const &ReferenceTimestamp) {
  while (StartByte + sizeof(DataHeader) < Packet.Length) {
    auto HeaderRaw =
        reinterpret_cast<const DataHeader *>(Packet.Data + StartByte);
    DataHeader Header(*HeaderRaw);
    Header.fixEndian();
    if (MODULE_HEADER != Header.MagicValue) {
      if (TWO_FILLER_BYTES == Header.MagicValue) {
        break;
      }
      throw ParserException(ParserException::Type::DATA_ABCD);
    }
    std::uint16_t NrOfSamples = (Header.Length - 20) / 2;
    if (StartByte + sizeof(DataHeader) + NrOfSamples * sizeof(std::uint16_t) +
            4 >
        Packet.Length) {
      throw ParserException(ParserException::Type::DATA_LENGTH);
    }
    auto CurrentChannelID = ChannelID{Source, Header.Channel};

    // Put the parsed data into the system
    SamplingRun *CurrentDataModule = PullDataModuleFromQueue(CurrentChannelID);
    if (CurrentDataModule != nullptr) {
      CurrentDataModule->Data.resize(NrOfSamples);
      CurrentDataModule->Identifier = CurrentChannelID;
      CurrentDataModule->StartTime = {Header.TimeStamp,
                                      ReferenceTimestamp.getClockMode()};
      CurrentDataModule->ReferenceTimestamp = ReferenceTimestamp;
      CurrentDataModule->OversamplingFactor = Header.Oversampling;
      auto ElementPointer = reinterpret_cast<const std::uint16_t *>(
          Packet.Data + StartByte + sizeof(DataHeader));
      for (int i = 0; i < NrOfSamples; ++i) {
        CurrentDataModule->Data[i] = ntohs(ElementPointer[i]);
      }

      if (not PushDataModuleToQueue(CurrentDataModule)) {
        // Things will be very problematic for us if we don't get rid of the
        // claimed data module, hence why we have a special exception for this
        // case.
        throw ModuleProcessingException(CurrentDataModule);
      }
    } else {
      throw ParserException(ParserException::Type::DATA_NO_MODULE);
    }
    StartByte += sizeof(DataHeader) + NrOfSamples * sizeof(std::uint16_t);
    const std::uint8_t *TrailerPointer = Packet.Data + StartByte;
    const std::uint32_t MagicValue = htonl(MODULE_TRAILER);
    if (std::memcmp(TrailerPointer, &MagicValue, sizeof(MagicValue)) != 0) {
      throw ParserException(ParserException::Type::DATA_BEEFCAFE);
    }
    StartByte += 4;
  }
  return StartByte;
}

TrailerInfo parseTrailer(const InData &Packet, std::uint32_t StartByte) {
  TrailerInfo ReturnInfo;
  auto FillerPointer =
      reinterpret_cast<const std::uint8_t *>(Packet.Data + StartByte);
  for (unsigned int i = 0; i < Packet.Length - StartByte - 4; i++) {
    if (FillerPointer[i] != FILLER_BYTE) {
      throw ParserException(ParserException::Type::TRAILER_0x55);
    }
    ++ReturnInfo.FillerBytes;
  }
  const std::uint8_t *TrailerPointer =
      Packet.Data + StartByte + ReturnInfo.FillerBytes;
  const std::uint32_t MagicValue = htonl(PACKET_TRAILER);
  if (0 != std::memcmp(TrailerPointer, &MagicValue, sizeof(MagicValue))) {
    throw ParserException(ParserException::Type::TRAILER_FEEDF00D);
  }
  return ReturnInfo;
}

IdleInfo parseIdle(const InData &Packet, std::uint32_t StartByte) {
  if (Packet.Length < StartByte + sizeof(IdleHeader)) {
    throw ParserException(ParserException::Type::IDLE_LENGTH);
  }
  IdleInfo ReturnData;
  auto *HeaderRaw =
      reinterpret_cast<const IdleHeader *>(Packet.Data + StartByte);
  IdleHeader Header(*HeaderRaw);
  Header.fixEndian();
  ReturnData.TimeStamp = Header.TimeStamp;
  return ReturnData;
}
