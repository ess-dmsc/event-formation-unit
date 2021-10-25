// Copyright (C) 2016 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <freia/geometry/Config.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

auto j2 = R"(
{
  "DoesNothing" : 0
}
)"_json;

auto NoDetector = R"(
{
  "WireChOffset" : 16,

  "Config" : [
    { "Ring" :  0, "CassOffset" :  1, "FENs" : 2},
    { "Ring" :  1, "CassOffset" :  5, "FENs" : 2},
    { "Ring" :  2, "CassOffset" :  9, "FENs" : 2},
    { "Ring" :  3, "CassOffset" : 13, "FENs" : 1},
    { "Ring" :  4, "CassOffset" : 15, "FENs" : 1},
    { "Ring" :  5, "CassOffset" : 17, "FENs" : 1},
    { "Ring" :  6, "CassOffset" : 19, "FENs" : 1},
    { "Ring" :  7, "CassOffset" : 21, "FENs" : 1},
    { "Ring" :  8, "CassOffset" : 23, "FENs" : 1},
    { "Ring" :  9, "CassOffset" : 25, "FENs" : 2},
    { "Ring" : 10, "CassOffset" : 29, "FENs" : 2}
  ],

  "MaxPulseTimeNS" : 357000000

}
)"_json;


auto InvalidDetector = R"(
{
  "Detector": "Freias",

  "WireChOffset" : 16,

  "Config" : [
    { "Ring" :  0, "CassOffset" :  1, "FENs" : 2},
    { "Ring" :  1, "CassOffset" :  5, "FENs" : 2},
    { "Ring" :  2, "CassOffset" :  9, "FENs" : 2},
    { "Ring" :  3, "CassOffset" : 13, "FENs" : 1},
    { "Ring" :  4, "CassOffset" : 15, "FENs" : 1},
    { "Ring" :  5, "CassOffset" : 17, "FENs" : 1},
    { "Ring" :  6, "CassOffset" : 19, "FENs" : 1},
    { "Ring" :  7, "CassOffset" : 21, "FENs" : 1},
    { "Ring" :  8, "CassOffset" : 23, "FENs" : 1},
    { "Ring" :  9, "CassOffset" : 25, "FENs" : 2},
    { "Ring" : 11, "CassOffset" : 29, "FENs" : 2}
  ],

  "MaxPulseTimeNS" : 357000000

}
)"_json;


auto InvalidRing = R"(
{
  "Detector": "Freia",

  "WireChOffset" : 16,

  "Config" : [
    { "Ring" :  0, "CassOffset" :  1, "FENs" : 2},
    { "Ring" :  1, "CassOffset" :  5, "FENs" : 2},
    { "Ring" :  2, "CassOffset" :  9, "FENs" : 2},
    { "Ring" :  3, "CassOffset" : 13, "FENs" : 1},
    { "Ring" :  4, "CassOffset" : 15, "FENs" : 1},
    { "Ring" :  5, "CassOffset" : 17, "FENs" : 1},
    { "Ring" :  6, "CassOffset" : 19, "FENs" : 1},
    { "Ring" :  7, "CassOffset" : 21, "FENs" : 1},
    { "Ring" :  8, "CassOffset" : 23, "FENs" : 1},
    { "Ring" :  9, "CassOffset" : 25, "FENs" : 2},
    { "Ring" : 11, "CassOffset" : 29, "FENs" : 2}
  ],

  "MaxPulseTimeNS" : 357000000
}
)"_json;

// std::string InvalidConfigFile{"deleteme_freia_invalid_config.json"};
// std::string InvalidConfigStr = R"(
// {
//   "Detector": "Freia",
//
//   "WireChOffset" : 16,
//
//   "Config" : [
//     { "Ring" :  0, "CassOffset" :  1, "FENs" : 2},
//     { "Ring" :  1, "CassOffset" :  5, "FENs" : 2},
//     { "Ring" :  2, "CassOffset" :  9, "FENs" : 2},
//     { "Ring" :  3, "CassOffset" : 13, "FENs" : 1},
//     { "Ring" :  4, "CassOffset" : 15, "FENs" : 1},
//     { "Rinx" :  5, "CassOffset" : 17, "FENs" : 1},
//     { "Ring" :  6, "CassOffset" : 19, "FENs" : 1},
//     { "Ring" :  7, "CassOffset" : 21, "FENs" : 1},
//     { "Ring" :  8, "CassOffset" : 23, "FENs" : 1},
//     { "Ring" :  9, "CassOffset" : 25, "FENs" : 2},
//     { "Ring" : 11, "CassOffset" : 29, "FENs" : 2}
//   ],
//
//   "MaxPulseTimeNS" : 357000000
// }
// )";

using namespace Freia;

class ConfigTest : public TestBase {
protected:
  Config config{"Freia", "config.json"};
  void SetUp() override {
    config.root = j2;
  }
  void TearDown() override {}
};

TEST_F(ConfigTest, Constructor) {
  ASSERT_EQ(config.NumPixels, 0);
  ASSERT_EQ(config.NumHybrids, 0);
}

// Compare calculated maxpixels and number of fens against
// ICD
struct RingCfg {
  uint8_t Ring;
  uint16_t FENs;
};



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

// TEST_F(ConfigTest, InvalidConfig) {
//   ASSERT_ANY_THROW(config = Config(InvalidConfigFile));
// }
//
// This table generated from the ICD and will be
// compared to the calculated values
// std::vector<RingCfg> FullConfig {
//   {  0, 2,  0,  0},
//   {  1, 2,  2,  8},
//   {  2, 2,  4, 16},
//   {  3, 1,  6, 24},
//   {  4, 1,  7, 28},
//   {  5, 1,  8, 32},
//   {  6, 1,  9, 36},
//   {  7, 1, 10, 40},
//   {  8, 1, 11, 44},
//   {  9, 2, 12, 48},
//   { 10, 2, 14, 56}
// };

// TEST_F(ConfigTest, FullInstrument) {
//   config = Config("Freia", FREIA_FULL);
//   config.loadAndApply();
//   ASSERT_EQ(config.NumPixels, 65536);
//   ASSERT_EQ(config.NumHybrids, 32);
//
//   for (const auto & Cfg : FullConfig) {
//     ASSERT_EQ(config.NumFENs[Cfg.Ring], Cfg.FENs);
//   }
// }


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();
  return RetVal;
}
