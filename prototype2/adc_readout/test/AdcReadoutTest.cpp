/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include "../AdcReadoutBase.h"
#include "TestUDPServer.h"
#include <gtest/gtest.h>
#include <array>
#include <trompeloeil.hpp>

class AdcReadoutStandIn : public AdcReadoutBase {
public:
  AdcReadoutStandIn(BaseSettings const &Settings,
                    AdcSettings const &ReadoutSettings)
      : AdcReadoutBase(Settings, ReadoutSettings){};
  ~AdcReadoutStandIn() override = default;
  using Detector::Threads;
  using AdcReadoutBase::AdcStats;
  using AdcReadoutBase::DataModuleQueues;
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

class AdcReadoutTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.DetectorAddress = "0.0.0.0";
    Settings.DetectorPort = GetPortNumber();
    ReadoutSettings.AltDetectorInterface = "0.0.0.0";
    ReadoutSettings.AltDetectorPort = GetPortNumber();
  }
  BaseSettings Settings;
  AdcSettings ReadoutSettings;
  static const int MaxPacketSize = 10000;
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
  };
  std::chrono::duration<std::int64_t, std::milli> SleepTime{50ms};
};

TEST_F(AdcReadoutTest, SinglePacketStats) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, 1470);
  std::this_thread::sleep_for(SleepTime);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 1470);
  EXPECT_EQ(Readout.AdcStats.parser_errors, 1);
}

TEST_F(AdcReadoutTest, SingleIdlePacket) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_idle.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                       PacketSize);
  std::this_thread::sleep_for(SleepTime);
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
  LoadPacketFile("test_packet_1.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                       PacketSize);
  std::this_thread::sleep_for(20ms);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(20ms);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 1470);
  EXPECT_EQ(Readout.AdcStats.parser_packets_total, 1);
  EXPECT_EQ(Readout.AdcStats.parser_packets_data, 1);
  EXPECT_EQ(Readout.AdcStats.processing_packets_lost, 0);
}

TEST_F(AdcReadoutTest, LazyThreadLaunching) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  EXPECT_EQ(Readout.Threads.size(), 1u);
  LoadPacketFile("test_packet_1.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                       PacketSize);
  std::this_thread::sleep_for(SleepTime);
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  EXPECT_EQ(Readout.Threads.size(), 2u);
  Readout.stopThreads();
}

TEST_F(AdcReadoutTest, DoubleReceiveTest) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_1.dat");
  TestUDPServer Server1(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                        PacketSize);
  TestUDPServer Server2(GetPortNumber(), ReadoutSettings.AltDetectorPort,
                        BufferPtr.data(), PacketSize);
  std::this_thread::sleep_for(SleepTime);
  Server1.startPacketTransmission(1, 100);
  Server2.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime * 2);
  EXPECT_EQ(Readout.Threads.size(), 3u);
  EXPECT_EQ(Readout.AdcStats.parser_packets_total, 2);
  Readout.stopThreads();
}

TEST_F(AdcReadoutTest, GlobalCounterError) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_1.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                       PacketSize);
  std::this_thread::sleep_for(SleepTime);
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
  Readout.startThreads();
  LoadPacketFile("test_packet_1.dat");
  std::this_thread::sleep_for(SleepTime);
  auto PacketHeadPointer = reinterpret_cast<PacketHeader *>(BufferPtr.data());
  {
    TestUDPServer Server1(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
                          PacketSize);
    Server1.startPacketTransmission(1, 100);
    std::this_thread::sleep_for(SleepTime);
  }
  PacketHeadPointer->fixEndian();
  PacketHeadPointer->GlobalCount++;
  PacketHeadPointer->fixEndian();
  {
    TestUDPServer Server2(GetPortNumber(), Settings.DetectorPort, BufferPtr.data(),
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
  AdcReadoutMock(BaseSettings const &Settings,
                 AdcSettings const &ReadoutSettings)
      : AdcReadoutBase(Settings, ReadoutSettings){};
  using Detector::Threads;
  using AdcReadoutBase::DataModuleQueues;
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
