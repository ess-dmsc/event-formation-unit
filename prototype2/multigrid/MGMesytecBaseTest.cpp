/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <string>
#include <common/DataSave.h>
#include <multigrid/MGMesytecBase.h>
#include <multigrid/mgmesytec/TestData.h>
#include <../prototype2/adc_readout/test/TestUDPServer.h>
#include <test/TestBase.h>

std::string mb16json = R"(
)";

class MGMesytecBaseStandIn : public MGMesytecBase {
public:
  MGMesytecBaseStandIn(BaseSettings Settings, struct MGMesytecSettings ReadoutSettings)
      : MGMesytecBase(Settings, ReadoutSettings){};
  ~MGMesytecBaseStandIn() = default;
  using Detector::Threads;
  using MGMesytecBase::mystats;
};

class MGMesytecBaseTest : public ::testing::Test {
public:
  virtual void SetUp() {
    //LocalSettings.ConfigFile = "MB16.json";
    Settings.DetectorRxBufferSize = 100000;
    Settings.MinimumMTU = 1500;
  }
  virtual void TearDown() {}

  BaseSettings Settings;
  MGMesytecSettings LocalSettings;
};

TEST_F(MGMesytecBaseTest, Constructor) {
  MGMesytecBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.mystats.rx_packets, 0);
  EXPECT_EQ(Readout.mystats.triggers, 0);
  EXPECT_EQ(Readout.mystats.readouts, 0);
}


TEST_F(MGMesytecBaseTest, DataReceive) {
  MGMesytecBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&ws2[0], ws2.size());
  Server.startPacketTransmission(1, 100);
  std::chrono::duration<std::int64_t, std::milli> SleepTime(200);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.mystats.rx_packets, 1);
  EXPECT_EQ(Readout.mystats.rx_bytes, ws2.size());
  EXPECT_EQ(Readout.mystats.readouts, 256); // 2 x 128 channels in ws2
}

int main(int argc, char **argv) {
  // std::string filename{"MB16.json"};
  // DataSave tempfile(filename, (void *)mb16json.c_str(), mb16json.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
