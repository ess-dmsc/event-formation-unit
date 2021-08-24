// Copyright (C) 2016 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <freia/geometry/Config.h>
#include <test/SaveBuffer.h>
#include <test/TestBase.h>


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
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
