// Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for TREXBase
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <string>
#include <trex/TREXBase.h>
#include <vector>


// clang-format off

std::vector<uint8_t> TestPacket {
              0x00, 0x00, // pad, v0
  0x45, 0x53, 0x53, 0x50, // 'E', 'S', 'S', type 0x50
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

class TREXBaseTest : public ::testing::Test {
public:
  void SetUp() override {
    Settings.ConfigFile = "trex.json";
    Settings.NoHwCheck = true;
  }
  void TearDown() override {}

  BaseSettings Settings;
};

TEST_F(TREXBaseTest, Constructor) {
  Trex::TrexBase Readout(Settings);
  EXPECT_EQ(Readout.ITCounters.RxPackets, 0);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0);
}

TEST_F(TREXBaseTest, DataReceive) {
  Trex::TrexBase Readout(Settings);

  writePacketToRxFIFO(Readout, TestPacket);

  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 2); // #readouts in TestPacket
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 2);
  Readout.stopThreads();
}

TEST_F(TREXBaseTest, DataReceiveBadHeader) {
  Trex::TrexBase Readout(Settings);

  TestPacket[0] = 0xff; // pad should be 0
  writePacketToRxFIFO(Readout, TestPacket);

  EXPECT_EQ(Readout.Counters.ErrorESSHeaders, 1);
  EXPECT_EQ(Readout.Counters.VMMStats.Readouts, 0); // no readouts: bad header
  EXPECT_EQ(Readout.Counters.VMMStats.DataReadouts, 0);
  Readout.stopThreads();
}

int main(int argc, char **argv) {
  std::string filename{"trex.json"};
  saveBuffer(filename, (void *)trexjson.c_str(), trexjson.size());

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
