// Copyright (C) 2018-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for MBCaenBase
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <string>
#include <vector>

std::vector<uint8_t> dummyreadout {
              0x00, 0x00, // pad, v0
  0x45, 0x53, 0x53, 0x48, // 'E', 'S', 'S', type 0x48
  0x46, 0x00, 0x17, 0x00, // len(0x005e), OQ23, TSrc0
  0x00, 0x00, 0x00, 0x00, // PT HI
  0x00, 0x00, 0x00, 0x00, // PT LO
  0x00, 0x00, 0x00, 0x00, // PPT HI
  0x00, 0x00, 0x00, 0x00, // PPT Lo
  0x08, 0x00, 0x00, 0x00, // Seq number 8

  // First readout
  0x01, 0x01, 0x14, 0x00,  // Data Header
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x00,  // GEO 0, TDC 0, VMM 0, CH 0

  // Second readout
  0x02, 0x02, 0x14, 0x00,  // Data Header
  0x01, 0x00, 0x00, 0x00,  // Time HI 1 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x00,  // GEO 0, TDC 0, VMM 0, CH 0
};

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
#include <test/TestUDPServer.h>
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
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
  Multiblade::CAENSettings LocalSettings;
};

TEST_F(CAENBaseTest, Constructor) {
  CAENBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0);
}


TEST_F(CAENBaseTest, DataReceive) {
  CAENBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&dummyreadout[0], dummyreadout.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, dummyreadout.size());
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 2); // number of readouts dummyreadout
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 2);
}

int main(int argc, char **argv) {
  std::string filename{"MB18Estia.json"};
  saveBuffer(filename, (void *)mb18estiajson.c_str(), mb18estiajson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
