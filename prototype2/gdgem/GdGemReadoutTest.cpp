/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include "GdGemBase.h"
//#include "TestUDPServer.h"
#include <gtest/gtest.h>
//#include <random>
//#include <trompeloeil.hpp>

class GdGemBaseStandIn : public GdGemBase {
public:
  GdGemBaseStandIn(BaseSettings Settings, struct NMXSettings ReadoutSettings)
      : GdGemBase(Settings, ReadoutSettings){};
  ~GdGemBaseStandIn() = default;
  using Detector::Threads;
  using GdGemBase::mystats;
};

class GdGemBaseTest : public ::testing::Test {
public:
  virtual void SetUp() {}
  virtual void TearDown() {}

  BaseSettings Settings;
  NMXSettings LocalSettings;
};

TEST_F(GdGemBaseTest, Constructor) {
  GdGemBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  // TestUDPServer Server(GetPortNumber(), Settings.DetectorPort, 1470);
  // Server.startPacketTransmission(1, 100);
  // std::chrono::duration<std::int64_t, std::milli> SleepTime(200);
  // std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.mystats.rx_packets, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
