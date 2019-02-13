/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcParse.h"
#include <cstdint>
#include <memory>

class DataPacket {
public:
  explicit DataPacket(size_t MaxPacketSize);
  bool addSamplingRun(void *DataPtr, size_t Bytes);
  std::pair<size_t, size_t> getBufferSizes() {
    return std::make_pair(Size, MaxSize);
  };
  std::pair<void *, size_t> getBuffer(std::uint16_t PacketCount,
                                      std::uint16_t ReadoutCount);
  void resetPacket();

private:
  std::unique_ptr<std::uint8_t[]> Buffer;
  PacketHeader *HeaderPtr;
  std::size_t Size{0};
  std::size_t MaxSize;
};
