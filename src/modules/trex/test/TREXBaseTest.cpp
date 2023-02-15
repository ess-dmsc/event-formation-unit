// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for TREXBase
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <string>
#include <vector>

// clang-format off

std::vector<uint8_t> dummyreadout {
              0x00, 0x00, // pad, v0
  0x45, 0x53, 0x53, 0x40, // 'E', 'S', 'S', type 0x48
  0x46, 0x00, 0x17, 0x00, // len(0x005e), OQ23, TSrc0
  0x00, 0x00, 0x00, 0x00, // PT HI
  0x00, 0x00, 0x00, 0x00, // PT LO
  0x00, 0x00, 0x00, 0x00, // PPT HI
  0x00, 0x00, 0x00, 0x00, // PPT Lo
  0x08, 0x00, 0x00, 0x00, // Seq number 8

   // First readout - plane Y - Grids
  0x00, 0x01, 0x14, 0x00,  // Data Header - Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x02, 0x3C,  // GEO 0, TDC 0, VMM 1, CH 60

  // Second readout - plane X & Z - Wires
  0x00, 0x01, 0x14, 0x00,  // Data Header, Ring 0, FEN 1
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x05, 0x00, 0x00, 0x00,  // Time LO 5 ticks
  0x00, 0x00, 0x00, 0x01,  // ADC 0x100
  0x00, 0x00, 0x00, 0x3C,  // GEO 0, TDC 0, VMM 0, CH 60
};

 std::string trexjson = R"(
  {
    "Detector": "TREX",
    "InstrumentGeometry" : "TREX",

    "Vessel_Config" : {
      "0": {"NumGrids": 140, "Rotation": false, "XOffset":   0},
      "1": {"NumGrids": 140, "Rotation": false, "XOffset":  12},
      "2": {"NumGrids": 140, "Rotation": false, "XOffset":  24},
      "3": {"NumGrids": 140, "Rotation": false, "XOffset":  36}
    },

    "Config" : [
      { "Ring" :  0, "VesselId": "0", "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
      { "Ring" :  0, "VesselId": "0", "FEN": 0, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000002"},
      { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000003"},
      { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000004"},
      { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000005"},
      { "Ring" :  0, "VesselId": "1", "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000006"},
      { "Ring" :  0, "VesselId": "1", "FEN": 2, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000007"},
      { "Ring" :  0, "VesselId": "1", "FEN": 2, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000008"},
      { "Ring" :  0, "VesselId": "1", "FEN": 3, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000009"},
      { "Ring" :  0, "VesselId": "1", "FEN": 3, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000010"},
      { "Ring" :  0, "VesselId": "1", "FEN": 3, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000011"},
      { "Ring" :  0, "VesselId": "2", "FEN": 4, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000012"},
      { "Ring" :  0, "VesselId": "2", "FEN": 4, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000013"},
      { "Ring" :  0, "VesselId": "2", "FEN": 4, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000014"},
      { "Ring" :  0, "VesselId": "2", "FEN": 5, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000015"},
      { "Ring" :  0, "VesselId": "2", "FEN": 5, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000016"},
      { "Ring" :  0, "VesselId": "2", "FEN": 5, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000017"},
      { "Ring" :  0, "VesselId": "3", "FEN": 6, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000018"},
      { "Ring" :  0, "VesselId": "3", "FEN": 6, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000019"},
      { "Ring" :  0, "VesselId": "3", "FEN": 6, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000020"},
      { "Ring" :  0, "VesselId": "3", "FEN": 7, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000021"},
      { "Ring" :  0, "VesselId": "3", "FEN": 7, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000022"},
      { "Ring" :  0, "VesselId": "3", "FEN": 7, "Hybrid" :  2, "HybridId" : "E5533333222222221111111100000023"}
    ],

    "MaxPulseTimeNS" : 71428570,
    "DefaultMinADC": 50,
    "MaxGridsPerEvent": 5,
    "SizeX": 12,
    "SizeY": 51,
    "SizeZ": 16
  }
)";

// clang-format on

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <common/testutils/TestUDPServer.h>
#include <trex/TREXBase.h>

class TREXBaseStandIn : public Trex::TrexBase {
public:
  TREXBaseStandIn(BaseSettings Settings) : Trex::TrexBase(Settings){};
  ~TREXBaseStandIn() = default;
  using Detector::Threads;
  using Trex::TrexBase::Counters;
};

class TREXBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.ConfigFile = "trex.json";
    Settings.RxSocketBufferSize = 100000;
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
};

TEST_F(TREXBaseTest, Constructor) {
  TREXBaseStandIn Readout(Settings);
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0);
}

TEST_F(TREXBaseTest, DataReceive) {
  TREXBaseStandIn Readout(Settings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&dummyreadout[0], dummyreadout.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, dummyreadout.size());
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts,
            2); // number of readouts dummyreadout
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 2);
}

TEST_F(TREXBaseTest, DataReceiveBadHeader) {
  TREXBaseStandIn Readout(Settings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  dummyreadout[0] = 0xff; // pad should be 0
  TestUDPServer Server(43126, Settings.DetectorPort,
                       (unsigned char *)&dummyreadout[0], dummyreadout.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.ITCounters.RxPackets, 1);
  EXPECT_EQ(Readout.ITCounters.RxBytes, dummyreadout.size());
  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts,
            0); // no readouts as header is bad
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 0);
}

int main(int argc, char **argv) {
  std::string filename{"trex.json"};
  saveBuffer(filename, (void *)trexjson.c_str(), trexjson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
