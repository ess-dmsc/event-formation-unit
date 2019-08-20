/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "DataPacket.h"
#include <cstring>

const std::uint32_t DATA_PACKET = 0x1111;

DataPacket::DataPacket(size_t MaxPacketSize)
    : Buffer(std::make_unique<std::uint8_t[]>(MaxPacketSize)),
      HeaderPtr(reinterpret_cast<PacketHeader *>(Buffer.get())),
      Size(sizeof(PacketHeader)), MaxSize(MaxPacketSize) {
  HeaderPtr->PacketType = DATA_PACKET;
}

bool DataPacket::addSamplingRun(void const *const DataPtr, size_t Bytes) {
  if (Size + Bytes + 4 > MaxSize) {
    return false;
  }
  std::memcpy(Buffer.get() + Size, DataPtr, Bytes);
  Size += Bytes;
  return true;
}

std::pair<void *, size_t> DataPacket::getBuffer(std::uint16_t PacketCount,
                                                std::uint16_t ReadoutCount) {
  std::uint32_t *TrailerPtr =
      reinterpret_cast<std::uint32_t *>(Buffer.get() + Size);
  auto TempValue = htonl(0xFEEDF00Du);
  std::memcpy(TrailerPtr, &TempValue, sizeof(TempValue));
  Size += 4;
  HeaderPtr->ReadoutCount = htons(ReadoutCount);
  HeaderPtr->ReadoutLength = htons(Size - 2);

  return std::make_pair(Buffer.get(), Size);
}

void DataPacket::resetPacket() { Size = sizeof(PacketHeader); }
