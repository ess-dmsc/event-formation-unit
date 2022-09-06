/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <common/testutils/TestBase.h>

#include <common/testutils/TestUDPServer.h>
#include <multigrid/MultigridBase.h>

// \todo use reference data instead
#include <multigrid/mesytec/test/TestData.h>

class MultigridBaseStandIn : public MultigridBase {
public:
  MultigridBaseStandIn(BaseSettings Settings,
                       MultigridSettings const &ReadoutSettings)
      : MultigridBase(Settings, ReadoutSettings){};
  ~MultigridBaseStandIn() = default;
  using Detector::Threads;
  using MultigridBase::Counters;
};

class MultigridBaseTest : public TestBase {
public:
  void SetUp() override {
    LocalSettings.ConfigFile = TEST_JSON_PATH "ILL_mappings.json";
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
  MultigridSettings LocalSettings;
};

TEST_F(MultigridBaseTest, Constructor) {
  MultigridBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.Counters.rx_packets, 0);
  EXPECT_EQ(Readout.Counters.rx_bytes, 0);
  EXPECT_EQ(Readout.Counters.tx_bytes, 0);
}

TEST_F(MultigridBaseTest, DataReceive) {
  MultigridBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> InitSleepTime{300};
  TestUDPServer Server(43126, Settings.DetectorPort, &ws4[0], ws4.size());
  std::this_thread::sleep_for(InitSleepTime);
  Server.startPacketTransmission(1, 1000);
  std::chrono::duration<std::int64_t, std::milli> SleepTime(1000);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.rx_packets, 1);
  EXPECT_EQ(Readout.Counters.rx_bytes, ws4.size());
  EXPECT_EQ(Readout.Counters.parser_discarded_bytes, 0);
  EXPECT_EQ(Readout.Counters.parser_triggers, 36);
  EXPECT_EQ(Readout.Counters.builder_glitch_rejects, 0);
  EXPECT_EQ(Readout.Counters.builder_filter_rejects, 0);
  EXPECT_EQ(Readout.Counters.builder_geometry_errors, 0);
  EXPECT_EQ(Readout.Counters.hits_total, 54);
  EXPECT_EQ(Readout.Counters.hits_bad_plane, 0);
  EXPECT_EQ(Readout.Counters.hits_time_seq_err, 0);
  EXPECT_EQ(Readout.Counters.hits_used, 44);
  EXPECT_EQ(Readout.Counters.pulses, 0);
  EXPECT_EQ(Readout.Counters.wire_clusters, 24);
  EXPECT_EQ(Readout.Counters.grid_clusters, 26);
  EXPECT_EQ(Readout.Counters.events_total, 23);
  EXPECT_EQ(Readout.Counters.events_multiplicity_rejects, 1);
  EXPECT_EQ(Readout.Counters.events_bad, 0);
  EXPECT_EQ(Readout.Counters.events_geometry_err, 0);
  EXPECT_EQ(Readout.Counters.tx_events, 22);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
