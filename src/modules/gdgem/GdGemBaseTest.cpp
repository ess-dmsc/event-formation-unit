/** Copyright (C) 2018-2020 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <gdgem/GdGemBase.h>
#include <adc_readout/test/TestUDPServer.h>
#include <gdgem/GdGemBaseTestData.h>
#include <test/TestBase.h>

class GdGemBaseStandIn : public GdGemBase {
public:
  GdGemBaseStandIn(BaseSettings Settings, struct NMXSettings ReadoutSettings)
      : GdGemBase(Settings, ReadoutSettings){};
  ~GdGemBaseStandIn() = default;
  using Detector::Threads;
  using GdGemBase::stats_;
};

class GdGemBaseTest : public TestBase {
public:
  void SetUp() override {
    LocalSettings.ConfigFile = TEST_JSON_PATH "vmm3.json";
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
  NMXSettings LocalSettings;
};

TEST_F(GdGemBaseTest, Constructor) {
  GdGemBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.stats_.RxPackets, 0);
}

// \todo this needs to be redone using the reference binary

TEST_F(GdGemBaseTest, GetCalibrationCmd) {
  GdGemBaseStandIn Readout(Settings, LocalSettings);
  const int OutputSize = 1000;
  unsigned int OutputBytes = 0;
  char Output[OutputSize];
  std::vector<std::string> badcmdlen = {"NMX_GET_CALIB"};
  int res = Readout.getCalibration(badcmdlen, Output, &OutputBytes);
  EXPECT_LT(res, 0);

  std::vector<std::string> goodcmd = {"NMX_GET_CALIB", "0", "0", "0"};
  res = Readout.getCalibration(goodcmd, Output, &OutputBytes);
  EXPECT_EQ(res, 0);
}

/// \todo This needs a little more work, better test data.
TEST_F(GdGemBaseTest, DataReceive) {
  Settings.UpdateIntervalSec = 0;
  GdGemBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime(400);
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&pkt44[0], pkt44.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.stats_.RxPackets, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
