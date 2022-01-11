// Copyright (C) 2016 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <cspec/geometry/Config.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

auto j2 = R"(
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
  "Detector": "CSPECs",
  "WireChOffset" : 16
}
)"_json;


auto InvalidRing = R"(
{
  "Detector": "CSPEC",

  "WireChOffset" : 16,

  "Config" : [
    { "Ring" :  0, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000000"},
    { "Ring" :  0, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
    { "Ring" :  0, "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000002"},
    { "Ring" :  0, "FEN": 2, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000003"},
    { "Ring" : 12, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000004"},
    { "Ring" :  1, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000005"}
  ]
}
)"_json;

std::string InvalidConfig = R"(
{
  "Detector": "CSPEC",

  "WireChOffset" : 16,

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
)";


auto DuplicateEntry = R"(
{
  "Detector": "CSPEC",

  "WireChOffset" : 16,

  "Config" : [
    { "Ring" :  0, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000000"},
    { "Ring" :  0, "FEN": 1, "Hybrid" :  1, "HybridId" : "E5533333222222221111111100000001"},
    { "Ring" :  0, "FEN": 2, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000002"},
    { "Ring" :  0, "FEN": 1, "Hybrid" :  0, "HybridId" : "E5533333222222221111111100000003"}
  ]
}
)"_json;

using namespace Cspec;

class ConfigTest : public TestBase {
protected:
  Config config{"CSPEC", "config.json"};
  void SetUp() override {
    config.root = j2;
  }
  void TearDown() override {}
};

TEST_F(ConfigTest, Constructor) {
  ASSERT_EQ(config.NumPixels, 0);
  //TODO run through Hybrids and check initialised
}

// TEST_F(ConfigTest, UninitialisedHybrids) {
//   ASSERT_ANY_THROW(config.getHybridId(0, 0, 0));
// }

TEST_F(ConfigTest, NoDetector) {
  config.root = NoDetector;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, InvalidDetector) {
  config.root = InvalidDetector;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, InvalidRing) {
  config.root = InvalidRing;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, InvalidConfig) {
  config.root = InvalidConfig;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, Duplicate) {
  config.root = DuplicateEntry;
  ASSERT_ANY_THROW(config.apply());
}


TEST_F(ConfigTest, FullInstrument) {
  config = Config("CSPEC", CSPEC_FULL);
  config.loadAndApply();
  // ASSERT_EQ(config.NumPixels, 65536);
  // ASSERT_EQ(config.NumHybrids, 32);

  //TODO, check correct Hybrids initialised
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();
  return RetVal;
}
