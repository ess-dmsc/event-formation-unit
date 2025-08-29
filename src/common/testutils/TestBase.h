// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Base class for all unit tests - provides colored print using MESSAGE
///
//===----------------------------------------------------------------------===//

#include <system_error>
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
void writePacketToRxFIFO(T &Base, std::vector<uint8_t> Packet) {

  /// First we load data packet into the ring buffer to ensure
  /// data is avaiable for processing thread when it's starts.
  unsigned int rxBufferIndex = Base.RxRingbuffer.getDataIndex();
  ASSERT_EQ(rxBufferIndex, 0);
  auto PacketSize = Packet.size();

  Base.RxRingbuffer.setDataLength(rxBufferIndex, PacketSize);
  auto DataPtr = Base.RxRingbuffer.getDataBuffer(rxBufferIndex);
  memcpy(DataPtr, (unsigned char *)&Packet[0], PacketSize);

  ASSERT_TRUE(Base.InputFifo.push(rxBufferIndex));
  Base.RxRingbuffer.getNextBuffer();

  /// we start all threads, but ring buffer already loaded with data
  Base.startThreads();

  /// we wait until processing have at least one idle cycle, to ensure data from
  /// ring buffer was processed
  waitForProcessing(Base);
}

/// \brief Wait until input and processing threads are in idle state
template <typename T> void waitForProcessing(T &Base) {
  while (Base.getInputCounters().RxIdle == 0) {
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
    static void saveToFile(const std::string &filename, void *buffer,
                           uint64_t datasize);
  };

#define GTEST_COUT std::cerr << "[ INFO     ] "
};