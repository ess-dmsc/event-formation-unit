/** Copyright (C) 2019 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <string>

#include <loki/LokiBase.h>
#include <../src/adc_readout/test/TestUDPServer.h>
#include <test/TestBase.h>

class LokiBaseStandIn : public Loki::LokiBase {
public:
  LokiBaseStandIn(BaseSettings Settings, struct Loki::LokiSettings ReadoutSettings)
      : Loki::LokiBase(Settings, ReadoutSettings){};
  ~LokiBaseStandIn() = default;
  using Detector::Threads;
  using Loki::LokiBase::Counters;
};

class LokiBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.DetectorRxBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
  Loki::LokiSettings LocalSettings;
};

TEST_F(LokiBaseTest, Constructor) {
  LokiBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
}


std::vector<uint8_t> TestPacket{0x00, 0x01, 0x02};

TEST_F(LokiBaseTest, DataReceive) {
  LokiBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&TestPacket[0], TestPacket.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, TestPacket.size());
  EXPECT_EQ(Readout.Counters.ReadoutsCount, 0); // no parser yet
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
