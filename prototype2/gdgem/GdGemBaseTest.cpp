/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <common/DataSave.h>
#include <gdgem/GdGemBase.h>
#include <../prototype2/adc_readout/test/TestUDPServer.h>
#include <gdgem/GdGemBaseTestData.h>
#include <test/TestBase.h>

class GdGemBaseStandIn : public GdGemBase {
public:
  GdGemBaseStandIn(BaseSettings Settings, struct NMXSettings ReadoutSettings)
      : GdGemBase(Settings, ReadoutSettings){};
  ~GdGemBaseStandIn() = default;
  using Detector::Threads;
  using GdGemBase::mystats;
};

class GdGemBaseTest : public TestBase {
public:
  virtual void SetUp() {
    LocalSettings.ConfigFile = TEST_JSON_PATH "vmm3.json";
    Settings.DetectorRxBufferSize = 100000;
    Settings.MinimumMTU = 1500;
  }
  virtual void TearDown() {}

  BaseSettings Settings;
  NMXSettings LocalSettings;
};

TEST_F(GdGemBaseTest, Constructor) {
  GdGemBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.mystats.rx_packets, 0);
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

  std::vector<std::string> badcmdarg = {"NMX_GET_CALIB", "700", "0", "0"};
  res = Readout.getCalibration(badcmdarg, Output, &OutputBytes);
  EXPECT_LT(res, 0);

  std::vector<std::string> goodcmd = {"NMX_GET_CALIB", "0", "0", "0"};
  res = Readout.getCalibration(goodcmd, Output, &OutputBytes);
  EXPECT_EQ(res, 0);
}


#if 0
TEST_F(GdGemBaseTest, DataReceive) {
  GdGemBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime(400);
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&pkt44[0], pkt44.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.mystats.rx_packets, 1);
  EXPECT_EQ(Readout.mystats.readouts, 53);
  EXPECT_EQ(Readout.mystats.readouts_error_bytes, 0);
}
#endif

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
