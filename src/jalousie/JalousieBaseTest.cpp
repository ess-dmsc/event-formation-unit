/** Copyright (C) 2019 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <string>

#include <jalousie/JalousieBase.h>
#include <../src/adc_readout/test/TestUDPServer.h>
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
    Settings.DetectorRxBufferSize = 100000;
    Settings.NoHwCheck = true;
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
  //TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&pkt145701[0], pkt145701.size());
  //Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Jalousie.stopThreads();
  EXPECT_EQ(Jalousie.Counters.RxPackets, 0);
  //EXPECT_EQ(Readout.Counters.RxBytes, pkt145701.size());
  //EXPECT_EQ(Readout.Counters.ReadoutsCount, 45); // number of readouts in pkt13_short
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
