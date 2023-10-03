// Copyright (C) 2016 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Base class for all unit tests - provides colored print using MESSAGE
///
//===----------------------------------------------------------------------===//

#include <thread>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wsign-compare"
#ifndef __clang__
#pragma GCC diagnostic ignored "-Waligned-new="
#endif
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

// For use i xxBaseTest
template <typename T>
void writePacketToRxFIFO(T & value, std::vector<uint8_t> Packet,
  std::chrono::duration<std::int64_t, std::milli> SleepTime) {
  value.startThreads();

  unsigned int rxBufferIndex = value.RxRingbuffer.getDataIndex();
  ASSERT_EQ(rxBufferIndex, 0);
  auto PacketSize = Packet.size();

  value.RxRingbuffer.setDataLength(rxBufferIndex, PacketSize);
  auto DataPtr = value.RxRingbuffer.getDataBuffer(rxBufferIndex);
  memcpy(DataPtr, (unsigned char *)&Packet[0], PacketSize);

  ASSERT_TRUE(value.InputFifo.push(rxBufferIndex));
  value.RxRingbuffer.getNextBuffer();

  std::this_thread::sleep_for(SleepTime);
}

namespace testing {
namespace internal {
enum GTestColor { COLOR_DEFAULT, COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

extern void ColoredPrintf(GTestColor color, const char *fmt, ...);
} // namespace internal
} // namespace testing

class TestBase : public ::testing::Test {
protected:
  class Message : public std::stringstream {
  public:
    static void saveToFile(std::string filename, void *buffer,
                           uint64_t datasize);
  };

#define GTEST_COUT std::cerr << "[ INFO     ] "
};
