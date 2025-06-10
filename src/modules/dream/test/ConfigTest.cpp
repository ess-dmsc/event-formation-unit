// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <dream/geometry/Config.h>

// clang-format off

// Invalid config file
auto InvalidCfgMissingDetectorField = R"(
{
  "NotDetector" : "InvalidField",

  "MaxPulseTimeDiffNS" : 50000
}
)"_json;

auto InvalidCfgWrongDetectorName = R"(
{
  "Detector" : "Freia",

  "MaxPulseTimeDiffNS" : 50000
}
)"_json;

auto InvalidRingConfParm = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 255, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;

auto InvalidFENConfParm = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 255, "Type" : "BwEndCap"}
  ]
}
)"_json;

auto InvalidTypeConfParm = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BadType"}
  ]
}
)"_json;

auto DuplicateConfParm = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"},
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;

auto MissingRing = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"},
    { "Ringz" : 4, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;


// finally a valid config file
auto ValidConfig = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;

auto ValidConfigDefaultPulseTime = R"(
{
  "Detector" : "DREAM",

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;

auto ValidConfigIndexes = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeDiffNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap", "Index" : 42, "Index2" : 84}
  ]
}
)"_json;

// clang-format on

using namespace Dream;

class DreamConfigTest : public TestBase {
protected:
  Config config{"config.json"}; // dummy filename, not used
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DreamConfigTest, Constructor) {
  ASSERT_EQ(config.MaxPulseTimeDiffNS, 5 * 71'428'571);
}

TEST_F(DreamConfigTest, NoConfigFile) {
  Config config2;
  ASSERT_THROW(config2.loadAndApply(), std::runtime_error);
}

TEST_F(DreamConfigTest, JsonFileNotExist) {
  ASSERT_THROW(config.loadAndApply(), std::runtime_error);
}

TEST_F(DreamConfigTest, InvalidConfig) {
  config.setRoot(InvalidCfgMissingDetectorField);
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(DreamConfigTest, InvalidConfigName) {
  config.setRoot(InvalidCfgWrongDetectorName);
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(DreamConfigTest, InvalidRingConfParm) {
  config.setRoot(InvalidRingConfParm);
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(DreamConfigTest, InvalidFENConfParm) {
  config.setRoot(InvalidFENConfParm);
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(DreamConfigTest, InvalidTypeConfParm) {
  config.setRoot(InvalidTypeConfParm);
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(DreamConfigTest, DuplicateConfParm) {
  config.setRoot(DuplicateConfParm);
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(DreamConfigTest, MissingRing) {
  config.setRoot(MissingRing);
  ASSERT_ANY_THROW(config.apply());
}

// Valid cfg file tests below

TEST_F(DreamConfigTest, ValidConfig) {
  ASSERT_FALSE(config.RMConfig[4][2].Initialised);

  config.setRoot(ValidConfig);
  config.apply();

  ASSERT_TRUE(config.RMConfig[4][2].Initialised);
  ASSERT_EQ(config.MaxPulseTimeDiffNS, 50000);
}

TEST_F(DreamConfigTest, ValidConfigDefaultPulseTime) {
  config.setRoot(ValidConfigDefaultPulseTime);
  config.MaxPulseTimeDiffNS = 1;
  config.apply();
  ASSERT_EQ(config.MaxPulseTimeDiffNS, 1);
}

TEST_F(DreamConfigTest, ValidIndexes) {
  ASSERT_FALSE(config.RMConfig[4][2].Initialised);
  ASSERT_EQ(config.RMConfig[4][2].P1.Index, 0);
  ASSERT_EQ(config.RMConfig[4][2].P2.Index, 0);

  config.setRoot(ValidConfigIndexes);
  config.apply();

  ASSERT_TRUE(config.RMConfig[4][2].Initialised);
  ASSERT_EQ(config.RMConfig[4][2].P1.Index, 42);
  ASSERT_EQ(config.RMConfig[4][2].P2.Index, 84);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
