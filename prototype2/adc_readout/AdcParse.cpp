/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief ADC data parsing code.
 */

#include "AdcParse.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <map>

ParserException::ParserException(std::string ErrorStr) : std::runtime_error(ErrorStr), ParserErrorType(Type::UNKNOWN), Error(ErrorStr) {
}

ParserException::ParserException(Type ErrorType) : std::runtime_error("Parser error of type " + std::to_string(static_cast<int>(ErrorType))), ParserErrorType(ErrorType) {
  
}

ParserException::Type ParserException::getErrorType() const {
  return ParserErrorType;
}

const char* ParserException::what() const noexcept {
  static std::map<ParserException::Type, std::string> ErrorTypeStrings = {
    {ParserException::Type::T_FEEDF00D, "Magic bytes (0xFEEDF00D) not found at the end of the packet."},
    {ParserException::Type::T_0x55, "Filler byte was not of value 0x55."},
    {ParserException::Type::D_BEEFCAFE, "Magic bytes (0xBEEFCAFE) was not at the end of the data module."},
    {ParserException::Type::D_LENGTH, "Packet size to short to hold expected number of samples."},
    {ParserException::Type::D_ABCD, "Data module did not start with magic bytes (0xABCD)."},
    {ParserException::Type::H_LENGTH, "Packet size was to short to hold header and expected payload."},
    {ParserException::Type::H_TYPE, "Indicated packet type is unknown."},
    {ParserException::Type::I_LENGTH, "Packet size was to short to hold idle time stamp."},
  };
  if (ParserException::Type::UNKNOWN == ParserErrorType) {
    return Error.c_str();
  }
  if (ErrorTypeStrings.find(ParserErrorType) == ErrorTypeStrings.end()) {
    return ("ParserException error string not defined for exception of type " + std::to_string(static_cast<int>(ParserErrorType))).c_str();
  }
  return ErrorTypeStrings.at(ParserErrorType).c_str();
}

HeaderInfo parseHeader(const InData &Packet) {
  HeaderInfo ReturnInfo;
  if (Packet.Length < sizeof(PacketHeader)) {
    throw ParserException(ParserException::Type::H_LENGTH);
  }
  const PacketHeader *HeaderRaw = reinterpret_cast<const PacketHeader*>(Packet.Data);
  PacketHeader Header(*HeaderRaw);
  Header.fixEndian();
  switch (Header.PacketType) {
    case 0x1111:
      ReturnInfo.Type = PacketType::Data;
      break;
    case 0x2222:
      ReturnInfo.Type = PacketType::Idle;
      break;
    default:
      throw ParserException(ParserException::Type::H_TYPE);
      break;
  }
  ReturnInfo.DataStart = sizeof(PacketHeader);
  ReturnInfo.GlobalCount = Header.GlobalCount;
  ReturnInfo.ReadoutCount = Header.ReadoutCount;
  if (Packet.Length - 2 != Header.ReadoutLength) {
    throw ParserException(ParserException::Type::H_LENGTH);
  }
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
      throw ParserException(ParserException::Type::D_ABCD);
    }
    std::uint16_t NrOfSamples = (Header.Length - 20) / 2;
    if (StartByte + sizeof(DataHeader) + NrOfSamples * sizeof(std::uint16_t) + 4> Packet.Length) {
      throw ParserException(ParserException::Type::D_LENGTH);
    }
    DataModule CurrentDataModule;
    CurrentDataModule.Data.resize(NrOfSamples);
    CurrentDataModule.Channel = Header.Channel;
    CurrentDataModule.TimeStampSeconds = Header.TimeStampSeconds;
    CurrentDataModule.TimeStampSecondsFrac = Header.TimeStampSecondsFrac;
    const std::uint16_t *ElementPointer = reinterpret_cast<const std::uint16_t*>(Packet.Data + StartByte + sizeof(DataHeader));
    for (int i = 0; i < NrOfSamples; ++i) {
      CurrentDataModule.Data[i] = ntohs(ElementPointer[i]);
    }
    StartByte += sizeof(DataHeader) + NrOfSamples * sizeof(std::uint16_t);
    const std::uint32_t *TrailerPointer = reinterpret_cast<const std::uint32_t*>(Packet.Data + StartByte);
    if (ntohl(*TrailerPointer) != 0xBEEFCAFE) {
      throw ParserException(ParserException::Type::D_BEEFCAFE);
    }
    StartByte += 4;
    ReturnData.Modules.emplace_back(CurrentDataModule);
  }
  ReturnData.FillerStart = StartByte;
  return ReturnData;
}

TrailerInfo parseTrailer(const InData &Packet, std::uint32_t StartByte) {
  TrailerInfo ReturnInfo;
  const std::uint8_t *FillerPointer = reinterpret_cast<const std::uint8_t*>(Packet.Data + StartByte);
  for (unsigned int i = 0; i < Packet.Length - StartByte - 4; i++) {
    if (FillerPointer[i] != 0x55) {
      throw ParserException(ParserException::Type::T_0x55);
    }
    ++ReturnInfo.FillerBytes;
  }
  const std::uint32_t *TrailerPointer = reinterpret_cast<const std::uint32_t*>(Packet.Data + StartByte + ReturnInfo.FillerBytes);
  if (ntohl(*TrailerPointer) != 0xFEEDF00D) {
    throw ParserException(ParserException::Type::T_FEEDF00D);
  }
  return ReturnInfo;
}

PacketData parsePacket(const InData &Packet) {
  HeaderInfo Header = parseHeader(Packet);
  PacketData ReturnData;
  ReturnData.GlobalCount = Header.GlobalCount;
  ReturnData.ReadoutCount = Header.ReadoutCount;
  if (PacketType::Data == Header.Type) {
    AdcData Data = parseData(Packet, Header.DataStart);
    parseTrailer(Packet, Data.FillerStart);
    ReturnData.Type = PacketType::Data;
    ReturnData.Modules = std::move(Data.Modules);
  } else if (PacketType::Idle == Header.Type) {
    IdleInfo Idle = parseIdle(Packet, Header.DataStart);
    ReturnData.Type = PacketType::Idle;
    ReturnData.IdleTimeStampSeconds = Idle.TimeStampSeconds;
    ReturnData.IdleTimeStampSecondsFrac = Idle.TimeStampSecondsFrac;
  }
  return ReturnData;
}

IdleInfo parseIdle(const InData &Packet, std::uint32_t StartByte) {
  if (Packet.Length < StartByte + sizeof(IdleHeader)) {
    throw ParserException(ParserException::Type::I_LENGTH);
  }
  IdleInfo ReturnData;
  const IdleHeader *HeaderRaw = reinterpret_cast<const IdleHeader*>(Packet.Data + StartByte);
  IdleHeader Header(*HeaderRaw);
  Header.fixEndian();
  ReturnData.TimeStampSeconds = Header.TimeStampSeconds;
  ReturnData.TimeStampSecondsFrac = Header.TimeStampSecondsFrac;
  return ReturnData;
}
