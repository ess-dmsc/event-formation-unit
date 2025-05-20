// Copyright (C) 2016 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <trex/geometry/Config.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

auto Empty = R"(
{
  "DoesNothing" : 0
}
)"_json;

auto NoDetector = R"(
{
  "WireChOffset" : 16
}
)"_json;

auto InvalidDetector = R"(
{
  "Detector": "TREXs",
  "WireChOffset" : 16
}
)"_json;

auto InvalidRing = R"(
{
  "Detector": "TREX",

  "Vessel_Config" : {
    "0":  {"NumGrids": 140, "Rotation": false, "XOffset":   0},
    "1":  {"NumGrids": 140, "Rotation": false, "XOffset":  12}
  },

  "Config" : [
    { "Ring" :  0, "VesselId": "0", "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000000"},
    { "Ring" :  0, "VesselId": "0", "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
    { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000002"},
    { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000003"},
    { "Ring" : 12, "VesselId": "1", "FEN": 0, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000004"},
    { "Ring" :  1, "VesselId": "1", "FEN": 0, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000005"}
  ]
}
)"_json;

auto InvalidConfig = R"(
{
  "Detector": "TREX",

  "Config" : [
    { "Ring" :  0, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000000"},
    { "Ring" :  0, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
    { "Ring" :  0, "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000002"},
    { "Ring" :  0, "FEN": 2, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000003"},
    { "Rinx" :  1, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000004"},
    { "Ring" :  1, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000005"}
  ],

  "MaxPulseTimeNS" : 357000000
}
)"_json;

auto DuplicateEntry = R"(
{
  "Detector": "TREX",
  "Vessel_Config" : {
      "0":  {"NumGrids": 140, "Rotation": false, "XOffset":   0},
      "1":  {"NumGrids": 140, "Rotation": false, "XOffset":  12}
  },
  "Config" : [
    { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000000"},
    { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
    { "Ring" :  0, "VesselId": "0", "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000002"},
    { "Ring" :  0, "VesselId": "0", "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000003"}
  ]
}
)"_json;

using namespace Trex;

class TrexConfigTest : public TestBase {
protected:
  Config config{"TREX", "config.json"};
  void SetUp() override { config.setRoot(Empty); }
  void TearDown() override {}
};

TEST_F(TrexConfigTest, Constructor) {
  ASSERT_EQ(config.NumPixels, 0);
  ASSERT_EQ(config.NumHybrids, 0);
}

TEST_F(TrexConfigTest, UninitialisedHybrids) {
  ASSERT_EQ(config.getHybrid(0, 0, 0).Initialised, false);
}

TEST_F(TrexConfigTest, NoDetector) {
  config.setRoot(NoDetector);
  ASSERT_ANY_THROW(config.applyVMM3Config());
}

TEST_F(TrexConfigTest, InvalidDetector) {
  config.setRoot(InvalidDetector);
  ASSERT_ANY_THROW(config.applyVMM3Config());
}

TEST_F(TrexConfigTest, InvalidRing) {
  config.setRoot(InvalidRing);
  ASSERT_ANY_THROW(config.applyVMM3Config());
}

TEST_F(TrexConfigTest, InvalidConfig) {
  config.setRoot(InvalidConfig);
  ASSERT_ANY_THROW(config.applyConfig());
}

TEST_F(TrexConfigTest, Duplicate) {
  config.setRoot(DuplicateEntry);
  ASSERT_ANY_THROW(config.applyVMM3Config());
}

TEST_F(TrexConfigTest, FullInstrument) {
  config = Config("TREX", TREX_FULL);
  config.loadAndApplyConfig();
  ASSERT_EQ(config.NumPixels, 838272);
  ASSERT_EQ(config.NumHybrids, 198);

  /// \todo, check correct Hybrids initialised
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();
  return RetVal;
}
