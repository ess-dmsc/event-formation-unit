/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <string>
#include <multiblade/mbcaen/DataParserTestData.h>

 std::string mb16json = R"(
 {
   "InstrumentGeometry": "Estia",

   "DigitizerConfig" : [
     { "index" : 0, "id" : 137 },
     { "index" : 1, "id" : 143 },
     { "index" : 2, "id" : 142 },
     { "index" : 3, "id" :  31 },
     { "index" : 4, "id" :  33 },
     { "index" : 5, "id" :  34 }
   ],

   "TimeTickNS": 16
 }
)";

#include <common/DataSave.h>
#include <multiblade/MBCaenBase.h>
#include <../prototype2/adc_readout/test/TestUDPServer.h>
#include <test/TestBase.h>

class MBCAENBaseStandIn : public MBCAENBase {
public:
  MBCAENBaseStandIn(BaseSettings Settings, struct MBCAENSettings ReadoutSettings)
      : MBCAENBase(Settings, ReadoutSettings){};
  ~MBCAENBaseStandIn() = default;
  using Detector::Threads;
  using MBCAENBase::mystats;
};

class MBCAENBaseTest : public ::testing::Test {
public:
  virtual void SetUp() {
    LocalSettings.ConfigFile = "MB16.json";
    Settings.DetectorRxBufferSize = 100000;
    Settings.MinimumMTU = 1500;
  }
  virtual void TearDown() {}

  BaseSettings Settings;
  MBCAENSettings LocalSettings;
};

TEST_F(MBCAENBaseTest, Constructor) {
  MBCAENBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.mystats.rx_packets, 0);
}


TEST_F(MBCAENBaseTest, DataReceive) {
  MBCAENBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&pkt145701[0], pkt145701.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.mystats.rx_packets, 1);
  EXPECT_EQ(Readout.mystats.rx_bytes, pkt145701.size());
  EXPECT_EQ(Readout.mystats.rx_readouts, 45); // number of readouts in pkt13_short
}

int main(int argc, char **argv) {
  std::string filename{"MB16.json"};
  DataSave tempfile(filename, (void *)mb16json.c_str(), mb16json.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
