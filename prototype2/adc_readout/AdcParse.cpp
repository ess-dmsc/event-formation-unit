/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief ADC data parsing code.
 */

#include "AdcParse.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <map>
#include <bitset>

ParserException::ParserException(std::string ErrorStr) : std::runtime_error(ErrorStr), ParserErrorType(Type::UNKNOWN), Error(ErrorStr) {
}

ParserException::ParserException(Type ErrorType) : std::runtime_error("Parser error of type " + std::to_string(static_cast<int>(ErrorType))), ParserErrorType(ErrorType) {
  
}

ParserException::Type ParserException::getErrorType() const {
  return ParserErrorType;
}

const char* ParserException::what() const noexcept {
  static std::map<ParserException::Type, std::string> ErrorTypeStrings = {
    {ParserException::Type::TRAILER_FEEDF00D, "Magic bytes (0xFEEDF00D) not found at the end of the packet."},
    {ParserException::Type::TRAILER_0x55, "Filler byte was not of value 0x55."},
    {ParserException::Type::DATA_BEEFCAFE, "Magic bytes (0xBEEFCAFE) was not at the end of the data module."},
    {ParserException::Type::DATA_LENGTH, "Packet size to short to hold expected number of samples."},
    {ParserException::Type::DATA_ABCD, "Data module did not start with magic bytes (0xABCD)."},
    {ParserException::Type::STREAM_ABCD, "Stream data module did not start with magic bytes (0xABCD)."},
    {ParserException::Type::STREAM_HEADER, "Stream module header does not fit in packet."},
    {ParserException::Type::STREAM_TYPE, "Type for data module was not correct (0xFFXX)."},
    {ParserException::Type::STREAM_OVERSAMPLING, "Unknown oversampling setting."},
    {ParserException::Type::STREAM_CHANNELS_MASK, "Too many active channels for current oversampling setting."},
    {ParserException::Type::STREAM_SIZE, "Packet to small to hold declared number of elements."},
    {ParserException::Type::STREAM_BEEFCAFE, "Magic bytes (0xBEEFCAFE) was not at the end of the data module."},
    {ParserException::Type::HEADER_LENGTH, "Packet size was to short to hold header and expected payload."},
    {ParserException::Type::HEADER_TYPE, "Indicated packet type is unknown."},
    {ParserException::Type::IDLE_LENGTH, "Packet size was to short to hold idle time stamp."},
  };
  if (ParserException::Type::UNKNOWN == ParserErrorType) {
    return Error.c_str();
  }
  if (ErrorTypeStrings.find(ParserErrorType) == ErrorTypeStrings.end()) {
    return "ParserException error string not defined for exceptions of this type.";
  }
  return ErrorTypeStrings.at(ParserErrorType).c_str();
}

HeaderInfo parseHeader(const InData &Packet) {
  HeaderInfo ReturnInfo;
  if (Packet.Length < sizeof(PacketHeader)) {
    throw ParserException(ParserException::Type::HEADER_LENGTH);
  }
  const PacketHeader *HeaderRaw = reinterpret_cast<const PacketHeader*>(Packet.Data);
  PacketHeader Header(*HeaderRaw);
  Header.fixEndian();
  if (0x1111 == Header.PacketType) {
    ReturnInfo.Type = PacketType::Data;
  } else if (0x2222 == Header.PacketType) {
    ReturnInfo.Type = PacketType::Idle;
  } else if (0x3300 == (Header.PacketType & 0xFF00)) {
    ReturnInfo.Type = PacketType::Stream;
  } else {
    throw ParserException(ParserException::Type::HEADER_TYPE);
  }
  ReturnInfo.DataStart = sizeof(PacketHeader);
  ReturnInfo.GlobalCount = Header.GlobalCount;
  ReturnInfo.ReadoutCount = Header.ReadoutCount;
  if (Packet.Length - 2 != Header.ReadoutLength) {
    throw ParserException(ParserException::Type::HEADER_LENGTH);
  }
  ReturnInfo.TypeValue = Header.PacketType;
  return ReturnInfo;
}

AdcData parseData(const InData &Packet, std::uint32_t StartByte) {
  AdcData ReturnData;
  while (StartByte + sizeof(DataHeader) < Packet.Length) {
    const DataHeader *HeaderRaw = reinterpret_cast<const DataHeader*>(Packet.Data + StartByte);
    DataHeader Header(*HeaderRaw);
    Header.fixEndian();
    if (0xABCD != Header.MagicValue) {
      if (0x5555 == Header.MagicValue) {
        break;
      }
      throw ParserException(ParserException::Type::DATA_ABCD);
    }
    std::uint16_t NrOfSamples = (Header.Length - 20) / 2;
    if (StartByte + sizeof(DataHeader) + NrOfSamples * sizeof(std::uint16_t) + 4> Packet.Length) {
      throw ParserException(ParserException::Type::DATA_LENGTH);
    }
    DataModule CurrentDataModule;
    CurrentDataModule.Data.resize(NrOfSamples);
    CurrentDataModule.Channel = Header.Channel;
    CurrentDataModule.TimeStamp = Header.TimeStamp;
    const std::uint16_t *ElementPointer = reinterpret_cast<const std::uint16_t*>(Packet.Data + StartByte + sizeof(DataHeader));
    for (int i = 0; i < NrOfSamples; ++i) {
      CurrentDataModule.Data[i] = ntohs(ElementPointer[i]);
    }
    StartByte += sizeof(DataHeader) + NrOfSamples * sizeof(std::uint16_t);
    const std::uint8_t *TrailerPointer = Packet.Data + StartByte;
    const std::uint32_t MagicValue = htonl(0xBEEFCAFE);
    if (std::memcmp(TrailerPointer, &MagicValue, sizeof(MagicValue)) != 0) {
      throw ParserException(ParserException::Type::DATA_BEEFCAFE);
    }
    StartByte += 4;
    ReturnData.Modules.emplace_back(CurrentDataModule);
  }
  ReturnData.FillerStart = StartByte;
  return ReturnData;
}

AdcData parseStreamData(const InData &Packet, std::uint32_t StartByte, std::uint16_t TypeValue) {
  AdcData ReturnData;
  StreamSetting CurrentSettings = parseStreamSettings(TypeValue);
  if (StartByte + sizeof(StreamHeader) > Packet.Length) {
    throw ParserException(ParserException::Type::STREAM_HEADER);
  }
  const StreamHeader *HeaderRaw = reinterpret_cast<const StreamHeader*>(Packet.Data + StartByte);
  StreamHeader Header(*HeaderRaw);
  Header.fixEndian();
  if (0xABCD != Header.MagicValue) {
    throw ParserException(ParserException::Type::STREAM_ABCD);
  }
  int NrOfElements = (Header.Length - 4 - sizeof(StreamHeader)) / 2;
  int NumberOfChannels = CurrentSettings.ChannelsActive.size();
  int ElementsPerChannel = NrOfElements / NumberOfChannels;
  for (int i = 0; i < NumberOfChannels; i++) {
    ReturnData.Modules.emplace_back(DataModule());
    ReturnData.Modules.at(i).Data.resize(ElementsPerChannel);
    ReturnData.Modules.at(i).OversamplingFactor = CurrentSettings.OversamplingFactor;
    ReturnData.Modules.at(i).TimeStamp = Header.TimeStamp;
  }
  if (StartByte + Header.Length + 4 != Packet.Length) {
    throw ParserException(ParserException::Type::STREAM_SIZE);
  }
  for (int j = 0; j < NrOfElements; j++) {
    const std::uint16_t *CurrentValue = reinterpret_cast<const std::uint16_t*>(Packet.Data + StartByte + sizeof(StreamHeader) + j * sizeof(std::uint16_t));
    
    ReturnData.Modules.at(j % NumberOfChannels).Data.at(j / NumberOfChannels) = ntohs(*CurrentValue);
  }
  ReturnData.FillerStart = StartByte + Header.Length;
  const std::uint8_t *TrailerPointer = Packet.Data + ReturnData.FillerStart - 4;
  const std::uint32_t MagicValue = htonl(0xBEEFCAFE);
  if (0 != std::memcmp(TrailerPointer, &MagicValue, sizeof(MagicValue))) {
    throw ParserException(ParserException::Type::STREAM_BEEFCAFE);
  }
  return ReturnData;
}

TrailerInfo parseTrailer(const InData &Packet, std::uint32_t StartByte) {
  TrailerInfo ReturnInfo;
  const std::uint8_t *FillerPointer = reinterpret_cast<const std::uint8_t*>(Packet.Data + StartByte);
  for (unsigned int i = 0; i < Packet.Length - StartByte - 4; i++) {
    if (FillerPointer[i] != 0x55) {
      throw ParserException(ParserException::Type::TRAILER_0x55);
    }
    ++ReturnInfo.FillerBytes;
  }
  const std::uint8_t *TrailerPointer = Packet.Data + StartByte + ReturnInfo.FillerBytes;
  const std::uint32_t MagicValue = htonl(0xFEEDF00D);
  if (0 != std::memcmp(TrailerPointer, &MagicValue, sizeof(MagicValue))) {
    throw ParserException(ParserException::Type::TRAILER_FEEDF00D);
  }
  return ReturnInfo;
}

PacketData parsePacket(const InData &Packet) {
  HeaderInfo Header = parseHeader(Packet);
  PacketData ReturnData;
  ReturnData.GlobalCount = Header.GlobalCount;
  ReturnData.ReadoutCount = Header.ReadoutCount;
  if (PacketType::Stream == Header.Type) {
    AdcData Data = parseStreamData(Packet, Header.DataStart, Header.TypeValue);
    parseTrailer(Packet, Data.FillerStart);
    ReturnData.Type = PacketType::Stream;
    ReturnData.Modules = std::move(Data.Modules);
  } else if (PacketType::Data == Header.Type) {
    AdcData Data = parseData(Packet, Header.DataStart);
    parseTrailer(Packet, Data.FillerStart);
    ReturnData.Type = PacketType::Data;
    ReturnData.Modules = std::move(Data.Modules);
  } else if (PacketType::Idle == Header.Type) {
    IdleInfo Idle = parseIdle(Packet, Header.DataStart);
    ReturnData.Type = PacketType::Idle;
    ReturnData.IdleTimeStamp = Idle.TimeStamp;
  }
  return ReturnData;
}

IdleInfo parseIdle(const InData &Packet, std::uint32_t StartByte) {
  if (Packet.Length < StartByte + sizeof(IdleHeader)) {
    throw ParserException(ParserException::Type::IDLE_LENGTH);
  }
  IdleInfo ReturnData;
  const IdleHeader *HeaderRaw = reinterpret_cast<const IdleHeader*>(Packet.Data + StartByte);
  IdleHeader Header(*HeaderRaw);
  Header.fixEndian();
  ReturnData.TimeStamp = Header.TimeStamp;
  return ReturnData;
}

StreamSetting parseStreamSettings(const std::uint16_t &SettingsRaw) {
  StreamSetting ReturnSetting;
  std::uint8_t LeadingByte = SettingsRaw >> 8;
  if (0x33 != LeadingByte) {
    throw ParserException(ParserException::Type::STREAM_TYPE);
  }
  int OversamplingSetting = SettingsRaw & 0x0F;
  if (OversamplingSetting > 4 or 3 == OversamplingSetting or 0 == OversamplingSetting) {
    throw ParserException(ParserException::Type::STREAM_OVERSAMPLING);
  }
  ReturnSetting.OversamplingFactor = OversamplingSetting;
  std::bitset<4> ActiveChannels((SettingsRaw & 0xF0) >> 4);
  if ((ActiveChannels.count() == 4 and 4 != OversamplingSetting) or (ActiveChannels.count() == 2 and OversamplingSetting < 2)) {
    throw ParserException(ParserException::Type::STREAM_CHANNELS_MASK);
  }
  for (size_t i = 0; i < ActiveChannels.size(); i++) {
    if (ActiveChannels.test(ActiveChannels.size() - 1 - i)){
      ReturnSetting.ChannelsActive.emplace_back(i);
    }
  }
  return ReturnSetting;
}
