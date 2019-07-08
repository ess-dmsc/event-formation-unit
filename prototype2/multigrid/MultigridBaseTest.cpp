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
#include "MultigridBase.h"

class MultigridBaseStandIn : public MultigridBase {
public:
  MultigridBaseStandIn(BaseSettings Settings, MultigridSettings const &ReadoutSettings)
      : MultigridBase(Settings, ReadoutSettings){};
  ~MultigridBaseStandIn() = default;
  using Detector::Threads;
  using MultigridBase::mystats;
};

class MultigridBaseTest : public TestBase {
public:
  void SetUp() override {
    LocalSettings.ConfigFile = TEST_JSON_PATH "ILL_mappings.json";
    Settings.DetectorRxBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
  MultigridSettings LocalSettings;
};

TEST_F(MultigridBaseTest, Constructor) {
  MultigridBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.mystats.rx_packets, 0);
  EXPECT_EQ(Readout.mystats.rx_bytes, 0);
  EXPECT_EQ(Readout.mystats.tx_bytes, 0);
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
  EXPECT_EQ(Readout.mystats.parser_discarded_bytes, 0);
  EXPECT_EQ(Readout.mystats.parser_triggers, 36);
  EXPECT_EQ(Readout.mystats.builder_glitch_rejects, 0);
  EXPECT_EQ(Readout.mystats.builder_filter_rejects, 0);
  EXPECT_EQ(Readout.mystats.builder_geometry_errors, 0);
  EXPECT_EQ(Readout.mystats.hits_total, 54);
  EXPECT_EQ(Readout.mystats.hits_bad_plane, 0);
  EXPECT_EQ(Readout.mystats.hits_time_seq_err, 0);
  EXPECT_EQ(Readout.mystats.hits_used, 44);
  EXPECT_EQ(Readout.mystats.pulses, 0);
  EXPECT_EQ(Readout.mystats.wire_clusters, 24);
  EXPECT_EQ(Readout.mystats.grid_clusters, 26);
  EXPECT_EQ(Readout.mystats.events_total, 23);
  EXPECT_EQ(Readout.mystats.events_multiplicity_rejects, 0);
  EXPECT_EQ(Readout.mystats.events_bad, 0);
  EXPECT_EQ(Readout.mystats.events_geometry_err, 0);
  EXPECT_EQ(Readout.mystats.events_time_err, 22);
  EXPECT_EQ(Readout.mystats.tx_events, 0);
  EXPECT_EQ(Readout.mystats.tx_bytes, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
