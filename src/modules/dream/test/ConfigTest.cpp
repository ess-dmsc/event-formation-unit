// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <dream/geometry/Config.h>

// clang-format off

// Invalid config file
auto InvalidCfgMissingDetectorField = R"(
{
  "NotDetector" : "InvalidField",

  "MaxPulseTimeNS" : 50000
}
)"_json;


// finally a valid config file
auto ValidConfig = R"(
{
  "Detector" : "DREAM",

  "MaxPulseTimeNS" : 50000,

  "Config" : [
    { "Ring" : 4, "FEN" : 2, "Type" : "BwEndCap"}
  ]
}
)"_json;


// clang-format on

using namespace Dream;

class ConfigTest : public TestBase {
protected:
  Config config{"config.json"}; // dummy filename, not used
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ConfigTest, Constructor) {
  ASSERT_EQ(config.MaxPulseTimeNS, 5 * 71'428'571);
}

TEST_F(ConfigTest, NoConfigFileTest) {
  Config config2;
  ASSERT_THROW(config2.loadAndApply(), std::runtime_error);
}

TEST_F(ConfigTest, JsonFileNotExistTest) {
  ASSERT_THROW(config.loadAndApply(), std::runtime_error);
}

TEST_F(ConfigTest, InvalidConfigTest) {
  config.root = InvalidCfgMissingDetectorField;
  ASSERT_ANY_THROW(config.apply());
}

TEST_F(ConfigTest, ValidConfigTest) {
  ASSERT_FALSE(config.RMConfig[4][2].Initialised);

  config.root = ValidConfig;
  config.apply();

  ASSERT_TRUE(config.RMConfig[4][2].Initialised);
  ASSERT_EQ(config.MaxPulseTimeNS, 50000);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
