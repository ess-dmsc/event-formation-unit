/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Unit tests.
 */

#include <gtest/gtest.h>
#include "../AdcReadoutBase.h"
#include "TestUDPServer.h"
#include <random>

#ifdef TROMPLELOEIL_AVAILABLE
#include <trompeloeil.hpp>
#endif

std::uint16_t GetPortNumber() {
  static std::uint16_t CurrentPortNumber = 0;
  if (0 == CurrentPortNumber) {
    std::random_device Device;
    std::mt19937 Generator(Device());
    std::uniform_int_distribution<std::uint16_t> Distribution (2048, 60000);
    CurrentPortNumber = Distribution(Generator);
  }
  return ++CurrentPortNumber;
}

class AdcReadoutStandIn : public AdcReadoutBase {
public:
  AdcReadoutStandIn(BaseSettings Settings, AdcSettings ReadoutSettings) : AdcReadoutBase(Settings, ReadoutSettings) {};
  using Detector::Threads;
  using AdcReadoutBase::Processors;
  using AdcReadoutBase::toParsingQueue;
  using AdcReadoutBase::AdcStats;
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
    PacketFile.read(reinterpret_cast<char*>(&BufferPtr), PacketSize);
    ASSERT_TRUE(PacketFile.good());
  }
};

class AdcReadoutTest : public ::testing::Test {
public:
  virtual void SetUp() {
    Settings.DetectorAddress = "localhost";
    Settings.DetectorPort = GetPortNumber();
  }
  BaseSettings Settings;
  AdcSettings ReadoutSettings;
  
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
    PacketFile.read(reinterpret_cast<char*>(&BufferPtr), PacketSize);
    ASSERT_TRUE(PacketFile.good());
  };
};

TEST_F(AdcReadoutTest, SinglePacketInQueue) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.Threads.at(0).thread = std::thread(Readout.Threads.at(0).func);
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, 100);
  Server.startPacketTransmission(1, 100);
  SpscBuffer::ElementPtr<InData> elem;
  EXPECT_TRUE(Readout.toParsingQueue.waitGetData(elem, 10000));
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 100);
  Readout.stopThreads();
}

TEST_F(AdcReadoutTest, SinglePacketStats) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, 1470);
  Server.startPacketTransmission(1, 100);
  std::chrono::duration<std::int64_t, std::milli> SleepTime(200);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 1470);
  EXPECT_EQ(Readout.AdcStats.parser_errors, 1);
}

TEST_F(AdcReadoutTest, SingleIdlePacket) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_idle.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr, PacketSize);
  Server.startPacketTransmission(1, 100);
  std::chrono::duration<std::int64_t, std::milli> SleepTime(200);
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
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr, PacketSize);
  Server.startPacketTransmission(1, 100);
  std::chrono::duration<std::int64_t, std::milli> SleepTime(200);
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
  LoadPacketFile("test_packet_1.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr, PacketSize);
  Server.startPacketTransmission(2, 100);
  std::chrono::duration<std::int64_t, std::milli> SleepTime(200);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 2*1470);
  EXPECT_EQ(Readout.AdcStats.parser_packets_total, 2);
  EXPECT_EQ(Readout.AdcStats.parser_errors, 0);
  EXPECT_EQ(Readout.AdcStats.parser_packets_data, 2);
  EXPECT_EQ(Readout.AdcStats.processing_packets_lost, 1);
}

TEST_F(AdcReadoutTest, GlobalCounterCorrect) {
  AdcReadoutStandIn Readout(Settings, ReadoutSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_1.dat");
  std::chrono::duration<std::int64_t, std::milli> SleepTime(50);
  auto PacketHeadPointer = reinterpret_cast<PacketHeader*>(BufferPtr);
  {
    TestUDPServer Server1(GetPortNumber(), Settings.DetectorPort, BufferPtr, PacketSize);
    Server1.startPacketTransmission(1, 100);
    std::this_thread::sleep_for(SleepTime);
  }
  PacketHeadPointer->fixEndian();
  PacketHeadPointer->GlobalCount++;
  PacketHeadPointer->fixEndian();
  {
    TestUDPServer Server2(GetPortNumber(), Settings.DetectorPort, BufferPtr, PacketSize);
    Server2.startPacketTransmission(1, 100);
    std::this_thread::sleep_for(SleepTime);
  }
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 2*1470);
  EXPECT_EQ(Readout.AdcStats.parser_packets_total, 2);
  EXPECT_EQ(Readout.AdcStats.parser_errors, 0);
  EXPECT_EQ(Readout.AdcStats.parser_packets_data, 2);
  EXPECT_EQ(Readout.AdcStats.processing_packets_lost, 0);
}

#ifdef TROMPLELOEIL_AVAILABLE
class AdcReadoutMock : public AdcReadoutBase {
public:
  AdcReadoutMock(BaseSettings Settings, AdcSettings ReadoutSettings) : AdcReadoutBase(Settings, ReadoutSettings) {};
  using Detector::Threads;
  using AdcReadoutBase::Processors;
  MAKE_MOCK0(inputThread, void(), override);
  MAKE_MOCK0(parsingThread, void(), override);
};

class AdcReadoutSimpleTest : public ::testing::Test {
public:
  BaseSettings Settings;
  AdcSettings ReadoutSettings;
};

TEST_F(AdcReadoutSimpleTest, StartProcessingThreads) {
  AdcReadoutMock Readout(Settings, ReadoutSettings);
  REQUIRE_CALL(Readout, inputThread()).TIMES(1);
  REQUIRE_CALL(Readout, parsingThread()).TIMES(1);
  Readout.startThreads();
  Readout.stopThreads();
}

TEST_F(AdcReadoutSimpleTest, DefaultProcessors) {
  AdcReadoutMock Readout(Settings, ReadoutSettings);
  EXPECT_EQ(Readout.Processors.size(), 0u);
}

TEST_F(AdcReadoutSimpleTest, ActivateProcessors) {
  ReadoutSettings.SerializeSamples = true;
  ReadoutSettings.PeakDetection = true;
  AdcReadoutMock Readout(Settings, ReadoutSettings);
  EXPECT_EQ(Readout.Processors.size(), 2u);
}

TEST_F(AdcReadoutSimpleTest, SetTimeStampLoc) {
  ReadoutSettings.SerializeSamples = true;
  ReadoutSettings.PeakDetection = false;
  std::vector<std::pair<std::string, TimeStampLocation>> TimeStampLocSettings{{"Start", TimeStampLocation::Start}, {"Middle", TimeStampLocation::Middle}, {"End", TimeStampLocation::End}};
  for (auto &Setting : TimeStampLocSettings) {
    ReadoutSettings.TimeStampLocation = Setting.first;
    AdcReadoutMock Readout(Settings, ReadoutSettings);
    const SampleProcessing *ProcessingPtr =  dynamic_cast<SampleProcessing*>(Readout.Processors.at(0).get());
    EXPECT_EQ(ProcessingPtr->getTimeStampLocation(), Setting.second);
  }
}

TEST_F(AdcReadoutSimpleTest, FailSetTimeStampLoc) {
  ReadoutSettings.SerializeSamples = true;
  ReadoutSettings.PeakDetection = false;
  ReadoutSettings.TimeStampLocation = "unknown";
  EXPECT_ANY_THROW(AdcReadoutMock(Settings, ReadoutSettings));
}

#endif
