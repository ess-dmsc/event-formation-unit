// Copyright (C) 2016 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Base class for all unit tests - provides colored print using MESSAGE
///
//===----------------------------------------------------------------------===//

#include <unistd.h>
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
void writePacketToRxFIFO(T & Base, std::vector<uint8_t> Packet) {
  Base.startThreads();

  unsigned int rxBufferIndex = Base.RxRingbuffer.getDataIndex();
  ASSERT_EQ(rxBufferIndex, 0);
  auto PacketSize = Packet.size();

  Base.RxRingbuffer.setDataLength(rxBufferIndex, PacketSize);
  auto DataPtr = Base.RxRingbuffer.getDataBuffer(rxBufferIndex);
  memcpy(DataPtr, (unsigned char *)&Packet[0], PacketSize);

  ASSERT_TRUE(Base.InputFifo.push(rxBufferIndex));
  Base.RxRingbuffer.getNextBuffer();

  while (Base.ITCounters.RxIdle == 0){
    usleep(100);
  }
  while (Base.Counters.ProcessingIdle == 0) {
    usleep(100);
  }
}

//
template <typename T>
void waitForProcessing(T & Base) {
  while (Base.ITCounters.RxIdle == 0){
    usleep(100);
  }
  while (Base.Counters.ProcessingIdle == 0) {
    usleep(100);
  }
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
