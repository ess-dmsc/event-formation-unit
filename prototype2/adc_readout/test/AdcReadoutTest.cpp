/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Unit tests.
 */

#include <gtest/gtest.h>
#include "../AdcReadoutCore.h"
#include "TestUDPServer.h"
#include <random>

class AdcReadoutStandIn : public AdcReadoutCore {
public:
  AdcReadoutStandIn(BaseSettings Settings) : AdcReadoutCore(Settings, AdcSettings) {};
  using Detector::Threads;
  using AdcReadoutCore::toParsingQueue;
  AdcSettingsStruct AdcSettings;
};

class AdcReadoutTest : public ::testing::Test {
public:
  virtual void SetUp() {
    Settings.DetectorAddress = "localhost";
    Settings.DetectorPort = 65535;
  }
  BaseSettings Settings;
};

TEST_F(AdcReadoutTest, StartThreads) {
  Settings.DetectorPort = 6666;
  AdcReadoutStandIn Readout(Settings);
  Readout.startThreads();
  EXPECT_EQ(Readout.Threads.size(), 2u);
  for (auto &t : Readout.Threads) {
    EXPECT_TRUE(t.thread.joinable());
  }
  Readout.stopThreads();
}

TEST_F(AdcReadoutTest, SinglePacket) {
  AdcReadoutStandIn Readout(Settings);
  Readout.Threads.at(0).thread = std::thread(Readout.Threads.at(0).func);
  TestUDPServer server(2048, 65535, 1, 100);
  SpscBuffer::ElementPtr<InData> elem;
  EXPECT_TRUE(Readout.toParsingQueue.waitGetData(elem, 10000));
  Readout.stopThreads();
}
