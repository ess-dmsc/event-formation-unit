// Copyright (C) 2016 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <caen/geometry/Config.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

std::string NotJsonFile{"deleteme_caen_notjson.json"};
std::string NotJsonStr = R"(
{
  Ceci n’est pas Json
)";

// Invalid config file: StrawResolution missing, invalid names:
// NotDetector
auto InvalidConfig = R"(
{
  "NotDetector": "LoKI4x8",

  "PanelConfig" : [
    { "Ring" : 0, "Vertical" :  true,  "TubesZ" : 4, "TubesN" : 8, "Offset" :      0 }
  ]
}
)"_json;

// Invalid config file: StrawResolution missing
auto InvalidConfigII = R"(
{
  "Detector": "loki",

  "PanelConfig" : [
    { "Ring" : 0, "Vertical" :  true,  "TubesZ" : 4, "TubesN" : 8, "Offset" :      0 }
  ]
}
)"_json;

// Invalid config file: Detector name is not LoKI
auto BadDetector = R"(
{
  "Detector": "LoKI4x8",

  "NotPanelConfig" : [
    { "Ring" : 0, "Vertical" :  true,  "TubesZ" : 4, "TubesN" : 8, "Offset" :      0 }
  ]
}
)"_json;

// Good configuration file
auto ValidConfig = R"(
{
  "Detector" : "loki",

  "StrawResolution" : 256,

  "PanelConfig" : [
    { "Bank" : 0, "Vertical" :  false,  "TubesZ" : 4, "TubesN" : 32, "StrawOffset" : 0    },
    { "Bank" : 1, "Vertical" :  false,  "TubesZ" : 4, "TubesN" : 24, "StrawOffset" : 896  }
  ]
}
)"_json;

using namespace Caen;

class CaenConfigTest : public TestBase {
protected:
  Config config;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(CaenConfigTest, Constructor) {
  ASSERT_EQ(config.Panels.size(), 0);
  ASSERT_EQ(config.Resolution, 0);
  ASSERT_EQ(config.NGroupsTotal, 0);
}

TEST_F(CaenConfigTest, NoConfigFile) {
  ASSERT_THROW(config = Config(""), std::runtime_error);
}

TEST_F(CaenConfigTest, JsonFileNotExist) {
  ASSERT_THROW(config = Config("/this_file_doesnt_exist"), std::runtime_error);
}

TEST_F(CaenConfigTest, NotJson) {
  ASSERT_ANY_THROW(config = Config(NotJsonFile));
  deleteFile(NotJsonFile);
}

TEST_F(CaenConfigTest, BadDetectorName) {
  config.root = BadDetector;
  ASSERT_ANY_THROW(config.parseConfig());
}

TEST_F(CaenConfigTest, InvalidConfig) {
  config.root = InvalidConfig;
  ASSERT_ANY_THROW(config.parseConfig());
}

TEST_F(CaenConfigTest, InvalidConfigII) {
  config.root = InvalidConfigII;
  ASSERT_ANY_THROW(config.parseConfig());
}

TEST_F(CaenConfigTest, ValidConfig) {
  config.root = ValidConfig;
  config.parseConfig();
  ASSERT_EQ(config.NGroupsTotal, (32 + 24) * 4);
  ASSERT_EQ(config.Panels.size(), 2);
}

// Validate full Loki instrument configuration (Loki.json)
// should match the definitions in the ICD
TEST_F(CaenConfigTest, CaenICDGeometryFull) {
  config = Config(CAEN_FULL);
  config.parseConfig();
  ASSERT_EQ(config.Panels[0].getGlobalUnitId(0, 0, 0), 0);
  ASSERT_EQ(config.Panels[1].getGlobalUnitId(0, 0, 0), 1568 * 32 / 56);
  ASSERT_EQ(config.Panels[2].getGlobalUnitId(0, 0, 0), 1568);
  ASSERT_EQ(config.Panels[3].getGlobalUnitId(0, 0, 0), 2016);
  ASSERT_EQ(config.Panels[4].getGlobalUnitId(0, 0, 0), 2352);
  ASSERT_EQ(config.Panels[5].getGlobalUnitId(0, 0, 0), 2800);
  ASSERT_EQ(config.Panels[6].getGlobalUnitId(0, 0, 0), 3136);
  ASSERT_EQ(config.Panels[7].getGlobalUnitId(0, 0, 0), 3920);
  ASSERT_EQ(config.Panels[8].getGlobalUnitId(0, 0, 0), 4816);
  ASSERT_EQ(config.Panels[9].getGlobalUnitId(0, 0, 0), 5376);
}

int main(int argc, char **argv) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
