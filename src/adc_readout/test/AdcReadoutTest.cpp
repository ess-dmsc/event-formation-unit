/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include "../AdcReadoutBase.h"
#include "TestUDPServer.h"
#include <array>
#include <gtest/gtest.h>
#include <trompeloeil.hpp>

class AdcReadoutStandIn : public AdcReadoutBase {
public:
  AdcReadoutStandIn(BaseSettings const &Settings,
                    AdcSettings const &ReadoutSettings)
      : AdcReadoutBase(Settings, ReadoutSettings){};
  ~AdcReadoutStandIn() override = default;
  using AdcReadoutBase::AdcStats;
  using AdcReadoutBase::DataModuleQueues;
  using Detector::Threads;
  static const int MaxPacketSize = 2048;

  std::array<std::uint8_t, MaxPacketSize> BufferPtr;
  int PacketSize{0};
  void LoadPacketFile(std::string const &FileName) {
    std::string PacketPath = TEST_PACKET_PATH;
    std::ifstream PacketFile(PacketPath + FileName, std::ios::binary);
    ASSERT_TRUE(PacketFile.good());
    PacketFile.seekg(0, std::ios::end);
    PacketSize = PacketFile.tellg();
    PacketFile.seekg(0, std::ios::beg);
    PacketFile.read(reinterpret_cast<char *>(BufferPtr.data()), PacketSize);
    ASSERT_TRUE(PacketFile.good());
  }
};

using namespace std::chrono_literals;

void LoadPacketFile(std::string const &FileName, std::uint8_t *BufferPtr,
                    int &BufferSize) {
  std::string PacketPath = TEST_PACKET_PATH;
  std::ifstream PacketFile(PacketPath + FileName, std::ios::binary);
  ASSERT_TRUE(PacketFile.good());
  PacketFile.seekg(0, std::ios::end);
  BufferSize = PacketFile.tellg();
  PacketFile.seekg(0, std::ios::beg);
  PacketFile.read(reinterpret_cast<char *>(BufferPtr), BufferSize);
  ASSERT_TRUE(PacketFile.good());
};

class AdcReadoutTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.DetectorAddress = "0.0.0.0";
    Settings.DetectorPort = GetPortNumber();
  }
  BaseSettings Settings;
  AdcSettings ReadoutSettings;
  static const int MaxPacketSize = 10000;
  std::array<std::uint8_t, MaxPacketSize> BufferPtr;
  int PacketSize{0};
  std::array<std::uint8_t, MaxPacketSize> ConfigBufferPtr;
  int ConfigPacketSize{0};
  std::chrono::duration<std::int64_t, std::milli> SleepTime{100ms};
};

TEST_F(AdcReadoutTest, DISABLED_SinglePacketStats) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, 1470);
  LoadPacketFile("test_packet_idle.dat", ConfigBufferPtr.data(),
                 ConfigPacketSize);
  Server.setConfigPacket(ConfigBufferPtr.data(), ConfigPacketSize);
  std::this_thread::sleep_for(SleepTime);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.InputBytesReceived, 1470);
  EXPECT_EQ(Readout.AdcStats.ParseErrors, 1);
}

TEST_F(AdcReadoutTest, DISABLED_SingleIdlePacket) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_idle.dat", BufferPtr.data(), PacketSize);
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                       PacketSize);
  LoadPacketFile("test_packet_idle.dat", ConfigBufferPtr.data(),
                 ConfigPacketSize);
  Server.setConfigPacket(ConfigBufferPtr.data(), ConfigPacketSize);
  std::this_thread::sleep_for(SleepTime);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.InputBytesReceived, PacketSize);
  EXPECT_EQ(Readout.AdcStats.ParserPacketsTotal, 1);
  EXPECT_EQ(Readout.AdcStats.ParserPacketsIdle, 1);
}

TEST_F(AdcReadoutTest, DISABLED_SingleDataPacket) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_1.dat", BufferPtr.data(), PacketSize);
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                       PacketSize);
  LoadPacketFile("test_packet_idle.dat", ConfigBufferPtr.data(),
                 ConfigPacketSize);
  Server.setConfigPacket(ConfigBufferPtr.data(), ConfigPacketSize);
  std::this_thread::sleep_for(20ms);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(20ms);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.InputBytesReceived, PacketSize);
  EXPECT_EQ(Readout.AdcStats.ParserPacketsTotal, 1);
  EXPECT_EQ(Readout.AdcStats.ParserPacketsData, 1);
}

TEST_F(AdcReadoutTest, DISABLED_LazyThreadLaunching) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  EXPECT_EQ(Readout.Threads.size(), 1u);
  LoadPacketFile("test_packet_1.dat", BufferPtr.data(), PacketSize);
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                       PacketSize);
  LoadPacketFile("test_packet_idle.dat", ConfigBufferPtr.data(),
                 ConfigPacketSize);
  Server.setConfigPacket(ConfigBufferPtr.data(), ConfigPacketSize);
  std::this_thread::sleep_for(SleepTime);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  EXPECT_EQ(Readout.Threads.size(), 2u);
  Readout.stopThreads();
}

TEST_F(AdcReadoutTest, DISABLED_ReceiveTest) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_1.dat", BufferPtr.data(), PacketSize);
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                       PacketSize);
  LoadPacketFile("test_packet_idle.dat", ConfigBufferPtr.data(),
                 ConfigPacketSize);
  Server.setConfigPacket(ConfigBufferPtr.data(), ConfigPacketSize);
  std::this_thread::sleep_for(SleepTime);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  EXPECT_EQ(Readout.Threads.size(), 2u);
  EXPECT_EQ(Readout.AdcStats.ParserPacketsTotal, 1);
  Readout.stopThreads();
}

TEST_F(AdcReadoutTest, DISABLED_GlobalCounterError) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_1.dat", BufferPtr.data(), PacketSize);
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                       PacketSize);
  LoadPacketFile("test_packet_idle.dat", ConfigBufferPtr.data(),
                 ConfigPacketSize);
  Server.setConfigPacket(ConfigBufferPtr.data(), ConfigPacketSize);
  std::this_thread::sleep_for(SleepTime);
  Server.startPacketTransmission(2, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.InputBytesReceived, 2 * PacketSize);
  EXPECT_EQ(Readout.AdcStats.ParserPacketsTotal, 2);
  EXPECT_EQ(Readout.AdcStats.ParseErrors, 0);
  EXPECT_EQ(Readout.AdcStats.ParserPacketsData, 2);
}

TEST_F(AdcReadoutTest, DISABLED_GlobalCounterCorrect) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_1.dat", BufferPtr.data(), PacketSize);
  std::this_thread::sleep_for(SleepTime);
  auto PacketHeadPointer = reinterpret_cast<PacketHeader *>(BufferPtr.data());
  {
    TestUDPServer Server1(GetPortNumber(), Settings.DetectorPort,
                          BufferPtr.data(), PacketSize);
    LoadPacketFile("test_packet_idle.dat", ConfigBufferPtr.data(),
                   ConfigPacketSize);
    Server1.setConfigPacket(ConfigBufferPtr.data(), ConfigPacketSize);
    Server1.startPacketTransmission(1, 100);
    std::this_thread::sleep_for(SleepTime);
  }
  PacketHeadPointer->fixEndian();
  PacketHeadPointer->fixEndian();
  {
    TestUDPServer Server2(GetPortNumber(), Settings.DetectorPort,
                          BufferPtr.data(), PacketSize);
    Server2.startPacketTransmission(1, 100);
    std::this_thread::sleep_for(SleepTime);
  }
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.InputBytesReceived, 2 * PacketSize);
  EXPECT_EQ(Readout.AdcStats.ParserPacketsTotal, 2);
  EXPECT_EQ(Readout.AdcStats.ParseErrors, 0);
  EXPECT_EQ(Readout.AdcStats.ParserPacketsData, 2);
}

using trompeloeil::_;

class AdcReadoutMock : public AdcReadoutBase {
public:
  AdcReadoutMock(BaseSettings const &Settings,
                 AdcSettings const &ReadoutSettings)
      : AdcReadoutBase(Settings, ReadoutSettings){};
  using AdcReadoutBase::DataModuleQueues;
  using Detector::Threads;
  MAKE_MOCK0(inputThread, void(), override);
  MAKE_MOCK2(processingThread, void(Queue &, std::shared_ptr<std::int64_t>),
             override);
};

class AdcReadoutSimpleTest : public ::testing::Test {
public:
  BaseSettings Settings;
  AdcSettings ReadoutSettings;
};

TEST_F(AdcReadoutSimpleTest, StartProcessingThreads) {
  AdcReadoutMock Readout(Settings, ReadoutSettings);
  REQUIRE_CALL(Readout, inputThread()).TIMES(1);
  REQUIRE_CALL(Readout, processingThread(_, _)).TIMES(0);
  Readout.startThreads();
  Readout.stopThreads();
}

TEST_F(AdcReadoutSimpleTest, DataQueues) {
  AdcReadoutMock Readout(Settings, ReadoutSettings);
  EXPECT_EQ(Readout.DataModuleQueues.size(), 0u);
}
