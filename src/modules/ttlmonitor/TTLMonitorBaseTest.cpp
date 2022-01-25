// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for TTLMonitorBase
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

 std::string freiajson = R"(
   {
     "Detector": "TTLMonitor",

     "WireChOffset" : 16,

     "Config" : [
       { "Ring" :  0, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000000"},
       { "Ring" :  0, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
       { "Ring" :  0, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000002"},
       { "Ring" :  0, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000003"},
       { "Ring" :  1, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000004"},
       { "Ring" :  1, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000005"},
       { "Ring" :  1, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000006"},
       { "Ring" :  1, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000007"},
       { "Ring" :  2, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000008"},
       { "Ring" :  2, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000009"},
       { "Ring" :  2, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000010"},
       { "Ring" :  2, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000011"},
       { "Ring" :  3, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000012"},
       { "Ring" :  3, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000013"},
       { "Ring" :  4, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000014"},
       { "Ring" :  4, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000015"},
       { "Ring" :  5, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000016"},
       { "Ring" :  5, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000017"},
       { "Ring" :  6, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000018"},
       { "Ring" :  6, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000019"},
       { "Ring" :  7, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000020"},
       { "Ring" :  7, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000021"},
       { "Ring" :  8, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000022"},
       { "Ring" :  8, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000023"},
       { "Ring" :  9, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000024"},
       { "Ring" :  9, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000025"},
       { "Ring" :  9, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000026"},
       { "Ring" :  9, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000027"},
       { "Ring" : 10, "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000028"},
       { "Ring" : 10, "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000029"},
       { "Ring" : 10, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000030"},
       { "Ring" : 10, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000031"}
     ],

     "MaxPulseTimeNS" : 71428570,
     "MaxGapWire"  : 0,
     "MaxGapStrip" : 0,
     "TimeBoxNs" : 2010
   }
)";

#include <common/testutils/SaveBuffer.h>
#include <ttlmonitor/TTLMonitorBase.h>
#include <common/testutils/TestUDPServer.h>
#include <common/testutils/TestBase.h>

class TTLMonitorBaseStandIn : public TTLMonitor::TTLMonitorBase {
public:
  TTLMonitorBaseStandIn(BaseSettings Settings, struct TTLMonitor::TTLMonitorSettings ReadoutSettings)
      : TTLMonitor::TTLMonitorBase(Settings, ReadoutSettings){};
  ~TTLMonitorBaseStandIn() = default;
  using Detector::Threads;
  using TTLMonitor::TTLMonitorBase::Counters;
};

class TTLMonitorBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    LocalSettings.ConfigFile = "TTLMonitor.json";
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
  TTLMonitor::TTLMonitorSettings LocalSettings;
};

TEST_F(TTLMonitorBaseTest, Constructor) {
  TTLMonitorBaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.Counters.RxPackets, 0);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0);
}


TEST_F(TTLMonitorBaseTest, DataReceive) {
  TTLMonitorBaseStandIn Readout(Settings, LocalSettings);
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

TEST_F(TTLMonitorBaseTest, DataReceiveBadHeader) {
  TTLMonitorBaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  dummyreadout[0] = 0xff; // pad should be 0
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&dummyreadout[0], dummyreadout.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.Counters.RxPackets, 1);
  EXPECT_EQ(Readout.Counters.RxBytes, dummyreadout.size());
  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0); // no readouts as header is bad
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 0);
}

int main(int argc, char **argv) {
  std::string filename{"TTLMonitor.json"};
  saveBuffer(filename, (void *)freiajson.c_str(), freiajson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
