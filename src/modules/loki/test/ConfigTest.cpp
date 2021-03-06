// Copyright (C) 2016 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <loki/geometry/Config.h>
#include <test/SaveBuffer.h>
#include <test/TestBase.h>

std::string NotJsonFile{"deleteme_loki_notjson.json"};
std::string NotJsonStr = R"(
{
  Ceci n’est pas Json
)";

// Invalid config file: StrawResolution missing, invalid names:
// NotDetector, NotPanelConfig
std::string InvalidConfigFile{"deleteme_loki_invalidconfig.json"};
std::string InvalidConfigStr = R"(
{
  "NotDetector": "LoKI4x8",

  "NotPanelConfig" : [
    { "Ring" : 0, "Vertical" :  true,  "TubesZ" : 4, "TubesN" : 8, "Offset" :      0 }
  ]
}
)";

// Good configuration file
std::string ValidConfigFile{"deleteme_loki_valid_conf.json"};
std::string ValidConfigStr = R"(
{
  "Detector" : "LoKI",

  "StrawResolution" : 256,

  "PanelConfig" : [
    { "Bank" : 0, "Vertical" :  false,  "TubesZ" : 4, "TubesN" : 32, "StrawOffset" : 0    },
    { "Bank" : 1, "Vertical" :  false,  "TubesZ" : 4, "TubesN" : 24, "StrawOffset" : 896  }
  ]
}
)";

using namespace Loki;

class ConfigTest : public TestBase {
protected:
  Config config;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ConfigTest, Constructor) {
  ASSERT_EQ(config.Panels.size(), 0);
  ASSERT_EQ(config.Resolution, 0);
  ASSERT_EQ(config.NTubesTotal, 0);
  ASSERT_EQ(config.getMaxPixel(), 0);
}

TEST_F(ConfigTest, NoConfigFile) {
  ASSERT_THROW(config = Config(""), std::runtime_error);
}

TEST_F(ConfigTest, JsonFileNotExist) {
  ASSERT_THROW(config = Config("/this_file_doesnt_exist"), std::runtime_error);
}

TEST_F(ConfigTest, NotJson) {
  ASSERT_ANY_THROW(config = Config(NotJsonFile));
  deleteFile(NotJsonFile);
}

TEST_F(ConfigTest, InvalidConfig) {
  ASSERT_ANY_THROW(config = Config(InvalidConfigFile));
  deleteFile(InvalidConfigFile);
}

TEST_F(ConfigTest, ValidConfig) {
  config = Config(ValidConfigFile);
  ASSERT_EQ(config.getMaxPixel(), (32+24)*4*7*256);
  ASSERT_EQ(config.NTubesTotal, (32+24)*4);
  ASSERT_EQ(config.Panels.size(), 2);
  deleteFile(ValidConfigFile);
}


int main(int argc, char **argv) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  saveBuffer(InvalidConfigFile, (void *)InvalidConfigStr.c_str(), InvalidConfigStr.size());
  saveBuffer(ValidConfigFile, (void *)ValidConfigStr.c_str(), ValidConfigStr.size());
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
