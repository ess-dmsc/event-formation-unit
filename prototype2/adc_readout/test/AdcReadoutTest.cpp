/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Unit tests.
 */

#include <gtest/gtest.h>
#include "../AdcReadoutCore.h"
#include "TestUDPServer.h"
#include <random>
#include <trompeloeil.hpp>

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

class AdcReadoutStandIn : public AdcReadoutCore {
public:
  AdcReadoutStandIn(BaseSettings Settings, AdcSettingsStruct AdcSettings) : AdcReadoutCore(Settings, AdcSettings) {};
  using Detector::Threads;
  using AdcReadoutCore::Processors;
  using AdcReadoutCore::toParsingQueue;
  using AdcReadoutCore::AdcStats;
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
  AdcSettingsStruct AdcSettings;
  
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
  AdcReadoutStandIn Readout(Settings, AdcSettings);
  Readout.Threads.at(0).thread = std::thread(Readout.Threads.at(0).func);
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, 100);
  Server.startPacketTransmission(1, 100);
  SpscBuffer::ElementPtr<InData> elem;
  EXPECT_TRUE(Readout.toParsingQueue.waitGetData(elem, 10000));
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 100);
  Readout.stopThreads();
}

TEST_F(AdcReadoutTest, SinglePacketStats) {
  AdcReadoutStandIn Readout(Settings, AdcSettings);
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
  AdcReadoutStandIn Readout(Settings, AdcSettings);
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
  AdcReadoutStandIn Readout(Settings, AdcSettings);
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

TEST_F(AdcReadoutTest, SingleStreamPacket) {
  AdcReadoutStandIn Readout(Settings, AdcSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_stream.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr, PacketSize);
  Server.startPacketTransmission(1, 100);
  std::chrono::duration<std::int64_t, std::milli> SleepTime(200);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 1470);
  EXPECT_EQ(Readout.AdcStats.parser_packets_total, 1);
  EXPECT_EQ(Readout.AdcStats.parser_errors, 0);
  EXPECT_EQ(Readout.AdcStats.parser_packets_stream, 1);
  EXPECT_EQ(Readout.AdcStats.processing_packets_lost, 0);
}

TEST_F(AdcReadoutTest, GlobalCounterError) {
  AdcReadoutStandIn Readout(Settings, AdcSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_stream.dat");
  TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, BufferPtr, PacketSize);
  Server.startPacketTransmission(2, 100);
  std::chrono::duration<std::int64_t, std::milli> SleepTime(200);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.AdcStats.input_bytes_received, 2*1470);
  EXPECT_EQ(Readout.AdcStats.parser_packets_total, 2);
  EXPECT_EQ(Readout.AdcStats.parser_errors, 0);
  EXPECT_EQ(Readout.AdcStats.parser_packets_stream, 2);
  EXPECT_EQ(Readout.AdcStats.processing_packets_lost, 1);
}

TEST_F(AdcReadoutTest, GlobalCounterCorrect) {
  AdcReadoutStandIn Readout(Settings, AdcSettings);
  Readout.startThreads();
  LoadPacketFile("test_packet_stream.dat");
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
  EXPECT_EQ(Readout.AdcStats.parser_packets_stream, 2);
  EXPECT_EQ(Readout.AdcStats.processing_packets_lost, 0);
}

class AdcReadoutMock : public AdcReadoutCore {
public:
  AdcReadoutMock(BaseSettings Settings, AdcSettingsStruct AdcSettings) : AdcReadoutCore(Settings, AdcSettings) {};
  using Detector::Threads;
  using AdcReadoutCore::Processors;
  MAKE_MOCK0(inputThread, void(), override);
  MAKE_MOCK0(parsingThread, void(), override);
};

class AdcReadoutSimpleTest : public ::testing::Test {
public:
  BaseSettings Settings;
  AdcSettingsStruct AdcSettings;
};

TEST_F(AdcReadoutSimpleTest, StartProcessingThreads) {
  AdcReadoutMock Readout(Settings, AdcSettings);
  REQUIRE_CALL(Readout, inputThread()).TIMES(1);
  REQUIRE_CALL(Readout, parsingThread()).TIMES(1);
  Readout.startThreads();
  Readout.stopThreads();
}

TEST_F(AdcReadoutSimpleTest, DefaultProcessors) {
  AdcReadoutMock Readout(Settings, AdcSettings);
  EXPECT_EQ(Readout.Processors.size(), 0u);
}

TEST_F(AdcReadoutSimpleTest, ActivateProcessors) {
  AdcSettings.SerializeSamples = true;
  AdcSettings.PeakDetection = true;
  AdcReadoutMock Readout(Settings, AdcSettings);
  EXPECT_EQ(Readout.Processors.size(), 2u);
}

TEST_F(AdcReadoutSimpleTest, SetTimeStampLoc) {
  AdcSettings.SerializeSamples = true;
  AdcSettings.PeakDetection = false;
  std::vector<std::pair<std::string, TimeStampLocation>> TimeStampLocSettings{{"Start", TimeStampLocation::Start}, {"Middle", TimeStampLocation::Middle}, {"End", TimeStampLocation::End}};
  for (auto &Setting : TimeStampLocSettings) {
    AdcSettings.TimeStampLocation = Setting.first;
    AdcReadoutMock Readout(Settings, AdcSettings);
    const SampleProcessing *ProcessingPtr =  dynamic_cast<SampleProcessing*>(Readout.Processors.at(0).get());
    EXPECT_EQ(ProcessingPtr->getTimeStampLocation(), Setting.second);
  }
}

TEST_F(AdcReadoutSimpleTest, FailSetTimeStampLoc) {
  AdcSettings.SerializeSamples = true;
  AdcSettings.PeakDetection = false;
  AdcSettings.TimeStampLocation = "unknown";
  EXPECT_ANY_THROW(AdcReadoutMock(Settings, AdcSettings));
}


