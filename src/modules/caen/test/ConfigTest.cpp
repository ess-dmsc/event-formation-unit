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
    { "Ring" : 0, "Vertical" :  true,  "GroupsZ" : 4, "GroupsN" : 8, "Offset" :      0 }
  ]
}
)"_json;

// Invalid config file: StrawResolution missing
auto InvalidConfigII = R"(
  {
    "Detector" : "loki",

    "GroupsZ" : 4,

    "ReadoutConstDelayNS" : 0,
    "MaxPulseTimeNS" : 357142855,
    "MaxTOFNS" : 1000000000,

    "Banks" : [
       {"Bank" : 0, "ID" : "bank0", "GroupsN" : 56, "YOffset" : 0}
    ],

    "Config" : [
      { "Ring" : 0, "Bank" : 0, "FENs" : 16, "FENOffset" :  0}
    ]
  }
)"_json;

// Invalid config file: Detector name is not LoKI
auto BadDetector = R"(
  {
    "Detector" : "lokixx",

    "Resolution" : 512,
    "GroupsZ" : 4,

    "ReadoutConstDelayNS" : 0,
    "MaxPulseTimeNS" : 357142855,
    "MaxTOFNS" : 1000000000,

    "Banks" : [
       {"Bank" : 0, "ID" : "bank0", "GroupsN" : 56, "YOffset" : 0}
    ],

    "Config" : [
      { "Ring" : 0, "Bank" : 0, "FENs" : 16, "FENOffset" :  0}
    ]
  }
)"_json;

// Good configuration file
auto ValidConfig = R"(
  {
    "Detector" : "loki",

    "Resolution" : 512,
    "GroupsZ" : 4,

    "ReadoutConstDelayNS" : 0,
    "MaxPulseTimeNS" : 357142855,
    "MaxTOFNS" : 1000000000,

    "Banks" : [
       {"Bank" : 0, "ID" : "bank0", "GroupsN" : 56, "YOffset" : 0}
    ],

    "Config" : [
      { "Ring" : 0, "Bank" : 0, "FENs" : 16, "FENOffset" :  0}
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
  ASSERT_EQ(config.LokiConf.Parms.TotalGroups, (32 + 24) * 4);
}

int main(int argc, char **argv) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
