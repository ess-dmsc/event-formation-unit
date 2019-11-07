/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <string>
#include <multiblade/caen/DataParserTestData.h>

 std::string mb18estiajson = R"(
 {
   "Detector": "MB18",

   "InstrumentGeometry": "Estia",

   "DigitizerConfig" : [
     { "index" : 0, "id" : 137 },
     { "index" : 1, "id" : 143 },
     { "index" : 2, "id" : 142 },
     { "index" : 3, "id" :  31 },
     { "index" : 4, "id" :  33 },
     { "index" : 5, "id" :  34 }
   ],

   "cassettes": 6,
   "wires":  32,
   "strips": 32,

   "TimeTickNS": 16
 }
)";

#include <test/SaveBuffer.h>
#include <multiblade/MBCaenBase.h>
#include <../src/adc_readout/test/TestUDPServer.h>
#include <test/TestBase.h>

class CAENBaseStandIn : public Multiblade::CAENBase {
public:
  CAENBaseStandIn(BaseSettings Settings, struct Multiblade::CAENSettings ReadoutSettings)
      : Multiblade::CAENBase(Settings, ReadoutSettings){};
  ~CAENBaseStandIn() = default;
  using Detector::Threads;
  using Multiblade::CAENBase::Counters;
};

class CAENBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    LocalSettings.ConfigFile = "MB18Estia.json";
    Settings.DetectorRxBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
  Multiblade::CAENSettings LocalSettings;
};

TEST_F(CAENBaseTest, Constructor) {
  CAENBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
}


TEST_F(CAENBaseTest, DataReceive) {
  CAENBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&pkt145701[0], pkt145701.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, pkt145701.size());
  EXPECT_EQ(Readout.Counters.ReadoutsCount, 45); // number of readouts in pkt13_short
}

int main(int argc, char **argv) {
  std::string filename{"MB18Estia.json"};
  saveBuffer(filename, (void *)mb18estiajson.c_str(), mb18estiajson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
