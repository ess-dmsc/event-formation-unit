/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include "../AdcReadoutBase.h"
#include "TestUDPServer.h"
#include <gtest/gtest.h>
#include <random>
#include <trompeloeil.hpp>

std::uint16_t GetPortNumber() {
  static std::uint16_t CurrentPortNumber = 0;
  if (0 == CurrentPortNumber) {
    std::random_device Device;
    std::mt19937 Generator(Device());
    std::uniform_int_distribution<std::uint16_t> Distribution(2048, 60000);
    CurrentPortNumber = Distribution(Generator);
  }
  return ++CurrentPortNumber;
}

class AdcReadoutStandIn : public AdcReadoutBase {
public:
  AdcReadoutStandIn(BaseSettings Settings, AdcSettings ReadoutSettings)
      : AdcReadoutBase(Settings, ReadoutSettings){};
  ~AdcReadoutStandIn() = default;
  using Detector::Threads;
  using AdcReadoutBase::AdcStats;
  using AdcReadoutBase::DataModuleQueues;
  static const int MaxPacketSize = 2048;
  std::uint8_t BufferPtr[MaxPacketSize];
  int PacketSize;
  void LoadPacketFile(std::string FileName) {
    std::string PacketPath = TEST_PACKET_PATH;
    std::ifstream PacketFile(PacketPath + FileName, std::ios::binary);
    ASSERT_TRUE(PacketFile.good());
    PacketFile.seekg(0, std::ios::end);
    PacketSize = PacketFile.tellg();
    PacketFile.seekg(0, std::ios::beg);
    PacketFile.read(reinterpret_cast<char *>(&BufferPtr), PacketSize);
    ASSERT_TRUE(PacketFile.good());
  }
};

class AdcReadoutTest : public ::testing::Test {
public:
  virtual void SetUp() {
    Settings.DetectorAddress = "127.0.0.1";
    Settings.DetectorPort = GetPortNumber();
  }
  BaseSettings Settings;
  AdcSettings ReadoutSettings;

  static const int MaxPacketSize = 10000;
  std::uint8_t BufferPtr[MaxPacketSize];
  int PacketSize;
  std::chrono::duration<std::int64_t, std::milli> InitSleepTime{300};
  std::chrono::duration<std::int64_t, std::milli> SleepTime{200};

  void LoadPacketFile(std::string FileName) {
    std::string PacketPath = TEST_PACKET_PATH;
    std::ifstream PacketFile(PacketPath + FileName, std::ios::binary);
    ASSERT_TRUE(PacketFile.good());
    PacketFile.seekg(0, std::ios::end);
    PacketSize = PacketFile.tellg();
    PacketFile.seekg(0, std::ios::beg);
    PacketFile.read(reinterpret_cast<char *>(&BufferPtr), PacketSize);
    ASSERT_TRUE(PacketFile.good());
  };
};

TEST_F(AdcReadoutTest, SinglePacketStats) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  std::this_thread::sleep_for(InitSleepTime);
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, 1470);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 1470);
  EXPECT_EQ(Readout.AdcStats.parser_errors, 1);
}

TEST_F(AdcReadoutTest, SingleIdlePacket) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  std::this_thread::sleep_for(InitSleepTime);
  LoadPacketFile("test_packet_idle.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr,
                       PacketSize);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 22);
  EXPECT_EQ(Readout.AdcStats.parser_packets_total, 1);
  EXPECT_EQ(Readout.AdcStats.parser_packets_idle, 1);
  EXPECT_EQ(Readout.AdcStats.processing_packets_lost, 0);
}

TEST_F(AdcReadoutTest, SingleDataPacket) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  std::this_thread::sleep_for(InitSleepTime);
  LoadPacketFile("test_packet_1.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr,
                       PacketSize);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 1470);
  EXPECT_EQ(Readout.AdcStats.parser_packets_total, 1);
  EXPECT_EQ(Readout.AdcStats.parser_packets_data, 1);
  EXPECT_EQ(Readout.AdcStats.processing_packets_lost, 0);
}

TEST_F(AdcReadoutTest, GlobalCounterError) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  std::this_thread::sleep_for(InitSleepTime);
  LoadPacketFile("test_packet_1.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr,
                       PacketSize);
  Server.startPacketTransmission(2, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 2 * 1470);
  EXPECT_EQ(Readout.AdcStats.parser_packets_total, 2);
  EXPECT_EQ(Readout.AdcStats.parser_errors, 0);
  EXPECT_EQ(Readout.AdcStats.parser_packets_data, 2);
  EXPECT_EQ(Readout.AdcStats.processing_packets_lost, 1);
}

TEST_F(AdcReadoutTest, GlobalCounterCorrect) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.Threads.at(0).thread = std::thread(Readout.Threads.at(0).func);
  Readout.Threads.at(1).thread = std::thread(Readout.Threads.at(1).func);
  std::this_thread::sleep_for(InitSleepTime);
  LoadPacketFile("test_packet_1.dat");
  std::chrono::duration<std::int64_t, std::milli> SleepTime(50);
  auto PacketHeadPointer = reinterpret_cast<PacketHeader *>(BufferPtr);
  {
    TestUDPServer Server1(GetPortNumber(), Settings.DetectorPort, BufferPtr,
                          PacketSize);
    Server1.startPacketTransmission(1, 100);
    std::this_thread::sleep_for(SleepTime);
  }
  PacketHeadPointer->fixEndian();
  PacketHeadPointer->GlobalCount++;
  PacketHeadPointer->fixEndian();
  {
    TestUDPServer Server2(GetPortNumber(), Settings.DetectorPort, BufferPtr,
                          PacketSize);
    Server2.startPacketTransmission(1, 100);
    std::this_thread::sleep_for(SleepTime);
  }
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 2 * 1470);
  EXPECT_EQ(Readout.AdcStats.parser_packets_total, 2);
  EXPECT_EQ(Readout.AdcStats.parser_errors, 0);
  EXPECT_EQ(Readout.AdcStats.parser_packets_data, 2);
  EXPECT_EQ(Readout.AdcStats.processing_packets_lost, 0);
}

using trompeloeil::_;

class AdcReadoutMock : public AdcReadoutBase {
public:
  AdcReadoutMock(BaseSettings Settings, AdcSettings ReadoutSettings)
      : AdcReadoutBase(Settings, ReadoutSettings){};
  using Detector::Threads;
  using AdcReadoutBase::DataModuleQueues;
  MAKE_MOCK0(inputThread, void(), override);
  MAKE_MOCK1(processingThread, void(Queue &), override);
};

class AdcReadoutSimpleTest : public ::testing::Test {
public:
  BaseSettings Settings;
  AdcSettings ReadoutSettings;
};

TEST_F(AdcReadoutSimpleTest, StartProcessingThreads) {
  AdcReadoutMock Readout(Settings, ReadoutSettings);
  REQUIRE_CALL(Readout, inputThread()).TIMES(1);
  REQUIRE_CALL(Readout, processingThread(_)).TIMES(4);
  Readout.startThreads();
  Readout.stopThreads();
}

TEST_F(AdcReadoutSimpleTest, DataQueues) {
  AdcReadoutMock Readout(Settings, ReadoutSettings);
  EXPECT_EQ(Readout.DataModuleQueues.size(), 4u);
}
