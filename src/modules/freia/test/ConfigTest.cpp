// Copyright (C) 2016 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <freia/geometry/Config.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>


std::string NotJsonFile{"deleteme_freia_notjson.json"};
std::string NotJsonStr = R"(
{
  Ceci n’est pas Json
)";


std::string NoDetectorFile{"deleteme_freia_nodetector.json"};
std::string NoDetectorStr = R"(
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
)";


std::string InvalidDetectorFile{"deleteme_freia_invaliddetector.json"};
std::string InvalidDetectorStr = R"(
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
)";


std::string InvalidRingFile{"deleteme_freia_invalid_ring.json"};
std::string InvalidRingStr = R"(
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
)";

std::string InvalidConfigFile{"deleteme_freia_invalid_config.json"};
std::string InvalidConfigStr = R"(
{
  "Detector": "Freia",

  "WireChOffset" : 16,

  "Config" : [
    { "Ring" :  0, "CassOffset" :  1, "FENs" : 2},
    { "Ring" :  1, "CassOffset" :  5, "FENs" : 2},
    { "Ring" :  2, "CassOffset" :  9, "FENs" : 2},
    { "Ring" :  3, "CassOffset" : 13, "FENs" : 1},
    { "Ring" :  4, "CassOffset" : 15, "FENs" : 1},
    { "Rinx" :  5, "CassOffset" : 17, "FENs" : 1},
    { "Ring" :  6, "CassOffset" : 19, "FENs" : 1},
    { "Ring" :  7, "CassOffset" : 21, "FENs" : 1},
    { "Ring" :  8, "CassOffset" : 23, "FENs" : 1},
    { "Ring" :  9, "CassOffset" : 25, "FENs" : 2},
    { "Ring" : 11, "CassOffset" : 29, "FENs" : 2}
  ],

  "MaxPulseTimeNS" : 357000000
}
)";

using namespace Freia;

class ConfigTest : public TestBase {
protected:
  Config config;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ConfigTest, Constructor) {
  ASSERT_EQ(config.NumPixels, 0);
  ASSERT_EQ(config.NumCassettes, 0);
}

// Compare calculated maxpixels and number of fens against
// ICD
struct RingCfg {
  uint8_t Ring;
  uint16_t FENs;
  uint16_t FENOffset;
  uint16_t VMMOffset;
};

TEST_F(ConfigTest, JsonError) {
  ASSERT_ANY_THROW(config = Config(NotJsonFile));
}

TEST_F(ConfigTest, NoDetector) {
  ASSERT_ANY_THROW(config = Config(NoDetectorFile));
}

TEST_F(ConfigTest, InvalidDetector) {
  ASSERT_ANY_THROW(config = Config(InvalidDetectorFile));
}

TEST_F(ConfigTest, InvalidRing) {
  ASSERT_ANY_THROW(config = Config(InvalidRingFile));
}

TEST_F(ConfigTest, InvalidConfig) {
  ASSERT_ANY_THROW(config = Config(InvalidConfigFile));
}

// This table generated from the ICD and will be
// compared to the calculated values
std::vector<RingCfg> FullConfig {
  {  0, 2,  0,  0},
  {  1, 2,  2,  8},
  {  2, 2,  4, 16},
  {  3, 1,  6, 24},
  {  4, 1,  7, 28},
  {  5, 1,  8, 32},
  {  6, 1,  9, 36},
  {  7, 1, 10, 40},
  {  8, 1, 11, 44},
  {  9, 2, 12, 48},
  { 10, 2, 14, 56}
};

TEST_F(ConfigTest, FullInstrument) {
  config = Config(FREIA_FULL);
  ASSERT_EQ(config.NumRings, 11);
  ASSERT_EQ(config.NumPixels, 65536);
  ASSERT_EQ(config.NumCassettes, 32);

  for (const auto & Cfg : FullConfig) {
    ASSERT_EQ(config.NumFens[Cfg.Ring], Cfg.FENs);
    ASSERT_EQ(config.FENOffset[Cfg.Ring], Cfg.FENOffset);
    ASSERT_EQ(config.VMMOffset[Cfg.Ring], Cfg.VMMOffset);
  }
}


int main(int argc, char **argv) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  saveBuffer(NoDetectorFile, (void *)NoDetectorStr.c_str(), NoDetectorStr.size());
  saveBuffer(InvalidDetectorFile, (void *)InvalidDetectorStr.c_str(), InvalidDetectorStr.size());
  saveBuffer(InvalidRingFile, (void *)InvalidRingStr.c_str(), InvalidRingStr.size());
  saveBuffer(InvalidConfigFile, (void *)InvalidConfigStr.c_str(), InvalidConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(NotJsonFile);
  deleteFile(NoDetectorFile);
  deleteFile(InvalidDetectorFile);
  deleteFile(InvalidRingFile);
  deleteFile(InvalidConfigFile);

  return RetVal;
}
