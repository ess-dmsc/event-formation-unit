// Copyright (C) 2016 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <loki/geometry/Config.h>

std::string NotJsonFile{"deleteme_loki_notjson.json"};
std::string NotJsonStr = R"(
{
  Ceci n’est pas Json
)";

// Invalid config file: StrawResolution missing, invalid names:
// NotDetector
std::string InvalidConfigFile{"deleteme_loki_invalidconfig.json"};
std::string InvalidConfigStr = R"(
{
  "NotDetector": "LoKI4x8",

  "PanelConfig" : [
    { "Ring" : 0, "Vertical" :  true,  "TubesZ" : 4, "TubesN" : 8, "Offset" :      0 }
  ]
}
)";

// Invalid config file: StrawResolution missing
std::string InvalidConfigIIFile{"deleteme_loki_invalidconfigii.json"};
std::string InvalidConfigIIStr = R"(
{
  "Detector": "LoKI",

  "PanelConfig" : [
    { "Ring" : 0, "Vertical" :  true,  "TubesZ" : 4, "TubesN" : 8, "Offset" :      0 }
  ]
}
)";

// Invalid config file: Detector name is not LoKI
std::string BadDetectorFile{"deleteme_loki_baddetector.json"};
std::string BadDetectorStr = R"(
{
  "Detector": "LoKI4x8",

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

TEST_F(ConfigTest, BadDetectorName) {
  ASSERT_ANY_THROW(config = Config(BadDetectorFile));
  deleteFile(BadDetectorFile);
}

TEST_F(ConfigTest, InvalidConfig) {
  ASSERT_ANY_THROW(config = Config(InvalidConfigFile));
  deleteFile(InvalidConfigFile);
}

TEST_F(ConfigTest, InvalidConfigII) {
  ASSERT_ANY_THROW(config = Config(InvalidConfigIIFile));
  deleteFile(InvalidConfigIIFile);
}

TEST_F(ConfigTest, ValidConfig) {
  config = Config(ValidConfigFile);
  ASSERT_EQ(config.getMaxPixel(), (32 + 24) * 4 * 7 * 256);
  ASSERT_EQ(config.NTubesTotal, (32 + 24) * 4);
  ASSERT_EQ(config.Panels.size(), 2);
  deleteFile(ValidConfigFile);
}

// Validate full instrument configuration (Loki.json)
// should match the definitions in the ICD
TEST_F(ConfigTest, LokiICDGeometryFull) {
  config = Config(LOKI_FULL);
  ASSERT_EQ(config.getMaxPixel(), 3211264);
  ASSERT_EQ(config.Panels[0].getGlobalStrawId(0, 0, 0), 0);
  ASSERT_EQ(config.Panels[1].getGlobalStrawId(0, 0, 0), 1568 * 32 / 56);
  ASSERT_EQ(config.Panels[2].getGlobalStrawId(0, 0, 0), 1568);
  ASSERT_EQ(config.Panels[3].getGlobalStrawId(0, 0, 0), 2016);
  ASSERT_EQ(config.Panels[4].getGlobalStrawId(0, 0, 0), 2352);
  ASSERT_EQ(config.Panels[5].getGlobalStrawId(0, 0, 0), 2800);
  ASSERT_EQ(config.Panels[6].getGlobalStrawId(0, 0, 0), 3136);
  ASSERT_EQ(config.Panels[7].getGlobalStrawId(0, 0, 0), 3920);
  ASSERT_EQ(config.Panels[8].getGlobalStrawId(0, 0, 0), 4816);
  ASSERT_EQ(config.Panels[9].getGlobalStrawId(0, 0, 0), 5376);
}

int main(int argc, char **argv) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  saveBuffer(InvalidConfigFile, (void *)InvalidConfigStr.c_str(),
             InvalidConfigStr.size());
  saveBuffer(InvalidConfigIIFile, (void *)InvalidConfigIIStr.c_str(),
             InvalidConfigIIStr.size());
  saveBuffer(BadDetectorFile, (void *)BadDetectorStr.c_str(),
             BadDetectorStr.size());
  saveBuffer(ValidConfigFile, (void *)ValidConfigStr.c_str(),
             ValidConfigStr.size());
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
