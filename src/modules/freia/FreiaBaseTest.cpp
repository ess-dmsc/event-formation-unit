// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for FreiaBase
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <string>
#include <vector>
// clang-format off
std::vector<uint8_t> dummyreadout{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x48, // 'E', 'S', 'S', type 0x48
    0x46, 0x00, 0x17, 0x00, // len(0x005e), OQ23, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x08, 0x00, 0x00, 0x00, // Seq number 8

    // First readout
    0x01, 0x01, 0x14, 0x00, // Data Header
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x00, 0x00, // GEO 0, TDC 0, VMM 0, CH 0

    // Second readout
    0x02, 0x02, 0x14, 0x00, // Data Header
    0x01, 0x00, 0x00, 0x00, // Time HI 1 s
    0x01, 0x00, 0x00, 0x00, // Time LO 1 tick
    0x00, 0x00, 0x00, 0x01, // ADC 0x100
    0x00, 0x00, 0x00, 0x00, // GEO 0, TDC 0, VMM 0, CH 0
};

std::string freiajson = R"(
   {
     "Detector": "Freia",

     "WireChOffset" : 16,

     "Config" : [
    { "CassetteNumber":  0, "Ring" :  0, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000000"},
    { "CassetteNumber":  1, "Ring" :  0, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
    { "CassetteNumber":  2, "Ring" :  0, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000002"},
    { "CassetteNumber":  3, "Ring" :  0, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000003"},
    { "CassetteNumber":  4, "Ring" :  1, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000004"},
    { "CassetteNumber":  5, "Ring" :  1, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000005"},
    { "CassetteNumber":  6, "Ring" :  1, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000006"},
    { "CassetteNumber":  7, "Ring" :  1, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000007"},
    { "CassetteNumber":  8, "Ring" :  2, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000008"},
    { "CassetteNumber":  9, "Ring" :  2, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000009"},
    { "CassetteNumber": 10, "Ring" :  2, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000010"},
    { "CassetteNumber": 11, "Ring" :  2, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000011"},
    { "CassetteNumber": 12, "Ring" :  3, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000012"},
    { "CassetteNumber": 13, "Ring" :  3, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000013"},
    { "CassetteNumber": 14, "Ring" :  4, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000014"},
    { "CassetteNumber": 15, "Ring" :  4, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000015"},
    { "CassetteNumber": 16, "Ring" :  5, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000016"},
    { "CassetteNumber": 17, "Ring" :  5, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000017"},
    { "CassetteNumber": 18, "Ring" :  6, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000018"},
    { "CassetteNumber": 19, "Ring" :  6, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000019"},
    { "CassetteNumber": 20, "Ring" :  7, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000020"},
    { "CassetteNumber": 21, "Ring" :  7, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000021"},
    { "CassetteNumber": 22, "Ring" :  8, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000022"},
    { "CassetteNumber": 23, "Ring" :  8, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000023"},
    { "CassetteNumber": 24, "Ring" :  9, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000024"},
    { "CassetteNumber": 25, "Ring" :  9, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000025"},
    { "CassetteNumber": 26, "Ring" :  9, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000026"},
    { "CassetteNumber": 27, "Ring" :  9, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000027"},
    { "CassetteNumber": 28, "Ring" : 10, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000028"},
    { "CassetteNumber": 29, "Ring" : 10, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000029"},
    { "CassetteNumber": 30, "Ring" : 10, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000030"},
    { "CassetteNumber": 31, "Ring" : 10, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000031"}
  ],

     "MaxPulseTimeNS" : 71428570,
     "MaxGapWire"  : 0,
     "MaxGapStrip" : 0,
     "TimeBoxNs" : 2010
   }
)";

//clang-format on

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <common/testutils/TestUDPServer.h>
#include <freia/FreiaBase.h>

class FreiaBaseStandIn : public Freia::FreiaBase {
public:
  FreiaBaseStandIn(BaseSettings Settings)
      : Freia::FreiaBase(Settings){};
  ~FreiaBaseStandIn() = default;
  using Detector::Threads;
  using Freia::FreiaBase::Counters;
};

class FreiaBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.ConfigFile = "Freia.json";
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
};

TEST_F(FreiaBaseTest, Constructor) {
  FreiaBaseStandIn Readout(Settings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0);
}

TEST_F(FreiaBaseTest, DataReceive) {
  FreiaBaseStandIn Readout(Settings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&dummyreadout[0], dummyreadout.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, dummyreadout.size());
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts,
            2); // number of readouts dummyreadout
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 2);
}

TEST_F(FreiaBaseTest, DataReceiveBadHeader) {
  FreiaBaseStandIn Readout(Settings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  dummyreadout[0] = 0xff; // pad should be 0
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&dummyreadout[0], dummyreadout.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, dummyreadout.size());
  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts,
            0); // no readouts as header is bad
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 0);
}

int main(int argc, char **argv) {
  std::string filename{"Freia.json"};
  saveBuffer(filename, (void *)freiajson.c_str(), freiajson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
