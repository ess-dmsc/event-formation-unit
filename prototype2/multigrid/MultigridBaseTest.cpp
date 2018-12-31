/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <multigrid/MultigridBase.h>
#include <../prototype2/adc_readout/test/TestUDPServer.h>
#include <test/TestBase.h>

// \todo use reference data
#include <multigrid/mesytec/TestData.h>

class MultigridBaseStandIn : public MultigridBase {
public:
  MultigridBaseStandIn(BaseSettings Settings, struct MultigridSettings ReadoutSettings)
      : MultigridBase(Settings, ReadoutSettings){};
  ~MultigridBaseStandIn() = default;
  using Detector::Threads;
  using MultigridBase::mystats;
};

class MultigridBaseTest : public TestBase {
public:
  virtual void SetUp() {
    LocalSettings.ConfigFile = TEST_JSON_PATH "ILL_mappings.json";
    Settings.DetectorRxBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  virtual void TearDown() {}

  BaseSettings Settings;
  MultigridSettings LocalSettings;
};

TEST_F(MultigridBaseTest, Constructor) {
  MultigridBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.mystats.rx_packets, 0);
  EXPECT_EQ(Readout.mystats.triggers, 0);
  EXPECT_EQ(Readout.mystats.readouts, 0);
}


TEST_F(MultigridBaseTest, DataReceive) {
  MultigridBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> InitSleepTime {300};
  TestUDPServer Server(43126, Settings.DetectorPort,
      &ws4[0], ws4.size());
  std::this_thread::sleep_for(InitSleepTime);
  Server.startPacketTransmission(1, 1000);
  std::chrono::duration<std::int64_t, std::milli> SleepTime(1000);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.mystats.rx_packets, 1);
  EXPECT_EQ(Readout.mystats.rx_bytes, ws4.size());
  EXPECT_EQ(Readout.mystats.readouts, 54); //
  EXPECT_EQ(Readout.mystats.events, 23);
  EXPECT_EQ(Readout.mystats.bus_glitches, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
