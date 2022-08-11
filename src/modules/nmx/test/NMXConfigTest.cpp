// Copyright (C) 2016 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <nmx/geometry/Config.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

auto j2 = R"(
{
  "DoesNothing" : 0
}
)"_json;

auto NoDetector = R"(
{
  "MaxSpanX" : 10
}
)"_json;

auto InvalidDetector = R"(
{
  "Detector": "NMXs",
  "MaxSpanX" : 10
}
)"_json;

auto InvalidRing = R"(
{
  "Detector" : "NMX",
  "InstrumentGeometry" : "NMX",
  "MaxSpanX" : 10,
  "MaxSpanY" : 10,
  "MaxGapX" : 2,
  "MaxGapY" : 2,
  "DefaultMinADC":50,
  "Config" : [
        {
          "Ring" :  0, 
          "FEN": 0, 
          "Hybrid" :  0, 
          "Plane" : 0,
          "Offset" : 0,
          "ReversedChannels" : true,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000000"
        },
        {
          "Ring" :  20, 
          "FEN": 0, 
          "Hybrid" :  1, 
          "Plane" : 0,
          "Offset" : 128,
          "ReversedChannels" : true,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000001"
        }
  ]
}
)"_json;

auto MinimalConfig = R"(
{
  "Detector": "NMX",
  "InstrumentGeometry" : "NMX",
  "Config" : [
    { "Ring" :  0, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000000"},
    { "Ring" :  0, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
    { "Ring" :  0, "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000002"},
    { "Ring" :  0, "FEN": 2, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000003"},
    { "Ring" :  1, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000004"},
    { "Ring" :  1, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000005"}
  ],

  "MaxPulseTimeNS" : 357000000
}
)"_json;

auto DuplicateEntry = R"(
{
  "Detector" : "NMX",
  "InstrumentGeometry" : "NMX",
  "MaxSpanX" : 10,
  "MaxSpanY" : 10,
  "MaxGapX" : 2,
  "MaxGapY" : 2,
  "DefaultMinADC":50,
  "Config" : [
        {
          "Ring" :  0, 
          "FEN": 0, 
          "Hybrid" :  0, 
          "Plane" : 0,
          "Offset" : 0,
          "ReversedChannels" : true,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000000"
        },
        {
          "Ring" :  20, 
          "FEN": 0, 
          "Hybrid" :  0, 
          "Plane" : 0,
          "Offset" : 128,
          "ReversedChannels" : true,
          "Panel" : 0,
          "HybridId" : "E5533333222222221111111100000001"
        }
  ]
}
)"_json;

using namespace Nmx;

class NMXConfigTest : public TestBase {
protected:
  Config config{"NMX", NMX_FULL};
  void SetUp() override { config.root = j2; }
  void TearDown() override {}
};

TEST_F(NMXConfigTest, Constructor) {
  ASSERT_EQ(config.NumPixels, 0);
  ASSERT_EQ(config.NumHybrids, 0);
}

TEST_F(NMXConfigTest, UninitialisedHybrids) {
  ASSERT_EQ(config.getHybrid(0, 0, 0).Initialised, false);
  // ASSERT_EQ(config.getHybrid(0, 1, 0).Initialised, true);
}

TEST_F(NMXConfigTest, NoDetector) {
  config.root = NoDetector;
  ASSERT_ANY_THROW(config.applyVMM3Config());
}

TEST_F(NMXConfigTest, MinimalConfig) {
  config.root = MinimalConfig;
  config.applyVMM3Config();
  ASSERT_EQ(config.getHybrid(0, 0, 0).Initialised, false);
  ASSERT_EQ(config.getHybrid(0, 1, 0).Initialised, true);
}

TEST_F(NMXConfigTest, InvalidDetector) {
  config.root = InvalidDetector;
  ASSERT_ANY_THROW(config.applyVMM3Config());
}

TEST_F(NMXConfigTest, InvalidRing) {
  config.root = InvalidRing;
  ASSERT_ANY_THROW(config.applyVMM3Config());
}

TEST_F(NMXConfigTest, Duplicate) {
  config.root = DuplicateEntry;
  ASSERT_ANY_THROW(config.applyVMM3Config());
}

TEST_F(NMXConfigTest, FullInstrument) {
  config = Config("NMX", NMX_FULL);
  config.loadAndApplyConfig();
  // ASSERT_EQ(config.NumPixels, 1638400);
  ASSERT_EQ(config.NumHybrids, 40);

  /// \todo, check correct Hybrids initialised
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();
  return RetVal;
}
