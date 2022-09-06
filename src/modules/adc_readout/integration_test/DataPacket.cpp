/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "DataPacket.h"
#include <cstring>

DataPacket::DataPacket(size_t MaxPacketSize)
    : Buffer(std::make_unique<std::uint8_t[]>(MaxPacketSize)),
      HeaderPtr(reinterpret_cast<PacketHeader *>(Buffer.get())),
      Size(sizeof(PacketHeader)), MaxSize(MaxPacketSize) {
  HeaderPtr->Type = PacketType::Data;
  HeaderPtr->Version = Protocol::VER_1;
  HeaderPtr->ClockMode = Clock::Clk_Ext;
  HeaderPtr->OversamplingFactor = 1;
}

bool DataPacket::addSamplingRun(void const *const DataPtr, size_t Bytes,
                                std::uint64_t ReferenceTime) {
  if (Size + Bytes + 4 > MaxSize) {
    return false;
  }
  if (FirstRefTimeInPacket == 0) {
    FirstRefTimeInPacket = ReferenceTime;
  }
  std::memcpy(Buffer.get() + Size, DataPtr, Bytes);
  Size += Bytes;
  return true;
}

std::pair<void *, size_t>
DataPacket::formatPacketForSend(std::uint16_t ReadoutCount) {
  std::uint32_t *TrailerPtr =
      reinterpret_cast<std::uint32_t *>(Buffer.get() + Size);
  auto TrailerVal = htonl(0xFEEDF00Du);
  std::memcpy(TrailerPtr, &TrailerVal, sizeof(TrailerVal));
  Size += 4;
  HeaderPtr->ReadoutCount = htons(ReadoutCount);
  HeaderPtr->ReadoutLength = htons(Size - 8);
  auto Time = TimeStamp(FirstRefTimeInPacket, TimeStamp::ClockMode::External);
  HeaderPtr->ReferenceTimeStamp = {Time.getSeconds(), Time.getSecondsFrac()};
  HeaderPtr->ReferenceTimeStamp.fixEndian();
  FirstRefTimeInPacket = 0;
  return std::make_pair(Buffer.get(), Size);
}

void DataPacket::resetPacket() { Size = sizeof(PacketHeader); }
