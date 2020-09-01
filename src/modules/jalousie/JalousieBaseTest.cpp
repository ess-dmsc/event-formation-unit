/** Copyright (C) 2019 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <string>

#include <jalousie/JalousieBase.h>
#include <adc_readout/test/TestUDPServer.h>
#include <jalousie/JalousieBaseTestData.h>
#include <test/SaveBuffer.h>
#include <test/TestBase.h>

class JalousieBaseStandIn : public Jalousie::JalousieBase {
public:
  JalousieBaseStandIn(BaseSettings Settings, struct Jalousie::CLISettings LocalJalousieSettings)
      : Jalousie::JalousieBase(Settings, LocalJalousieSettings){};
  ~JalousieBaseStandIn() = default;
  using Detector::Threads;
  using Jalousie::JalousieBase::Counters;
};

class JalousieBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
    LocalSettings.ConfigFile = TEST_JSON_PATH "v20_mappings.json";
  }
  void TearDown() override {}

  BaseSettings Settings;
  Jalousie::CLISettings LocalSettings;
};

TEST_F(JalousieBaseTest, Constructor) {
  JalousieBaseStandIn Jalousie(Settings, LocalSettings);
  EXPECT_EQ(Jalousie.Counters.RxPackets, 0);
}

TEST_F(JalousieBaseTest, DataReceive) {
  JalousieBaseStandIn Jalousie(Settings, LocalSettings);
  Jalousie.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&DummyJalousieData[0],
    DummyJalousieData.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Jalousie.stopThreads();
  EXPECT_EQ(Jalousie.Counters.RxPackets, 1);
  EXPECT_EQ(Jalousie.Counters.RxBytes, DummyJalousieData.size());
  EXPECT_EQ(Jalousie.Counters.ReadoutCount, 4);
  EXPECT_EQ(Jalousie.Counters.BadModuleId, 1);
  EXPECT_EQ(Jalousie.Counters.MappingErrors, 1);
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
