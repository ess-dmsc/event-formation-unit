// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for SoNDeBase
///
//===----------------------------------------------------------------------===//

#include <sonde/SoNDeBase.h>
#include <sonde/SoNDeBaseTestData.h>
#include <test/TestUDPServer.h>
#include <test/TestBase.h>

class SONDEIDEABaseStandIn : public SONDEIDEABase {
public:
  SONDEIDEABaseStandIn(BaseSettings Settings, struct SoNDeSettings ReadoutSettings)
      : SONDEIDEABase(Settings, ReadoutSettings){};
  ~SONDEIDEABaseStandIn() {};
  using Detector::Threads;
  using SONDEIDEABase::mystats;
};

class SoNDeBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
    LocalSettings.fileprefix = "sonde_";
  }
  void TearDown() override {}

  BaseSettings Settings;
  SoNDeSettings LocalSettings;
};

TEST_F(SoNDeBaseTest, Constructor) {
  SONDEIDEABaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.mystats.rx_packets, 0);
  EXPECT_EQ(Readout.mystats.rx_events, 0);
  EXPECT_EQ(Readout.mystats.tx_bytes, 0);
}

TEST_F(SoNDeBaseTest, DataReceive) {
  SONDEIDEABaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&sondedata[0], sondedata.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime * 3); // To ensure periodic produce is called
  Readout.stopThreads();
  EXPECT_EQ(Readout.mystats.rx_packets, 1);
  EXPECT_EQ(Readout.mystats.rx_bytes, sondedata.size());
  EXPECT_EQ(Readout.mystats.rx_events, 250); // number of readouts == events
  EXPECT_EQ(Readout.mystats.tx_bytes, 992080); // regardless of number of events inside!
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
