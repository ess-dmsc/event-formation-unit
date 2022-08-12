// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <dream/geometry/Config.h>

std::string NotJsonFile{"deleteme_dream_notjson.json"};
std::string NotJsonStr = R"(
{
  Ceci n’est pas Json
)";

// Invalid config file
std::string InvalidConfigFile{"deleteme_dream_invalidconfig.json"};
std::string InvalidConfigStr = R"(
{
  "NotDetector": "InvalidField",

  "MaxPulseTimeNS" : 5000000000
}
)";

// // Good configuration file
// std::string ValidConfigFile{"deleteme_loki_valid_conf.json"};
// std::string ValidConfigStr = R"(
// {
//   "Detector": "DREAM",
//
//   "MaxPulseTimeNS" : 5000000000
// }
// )";

using namespace Dream;

class ConfigTest : public TestBase {
protected:
  Config config;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ConfigTest, Constructor) {
  ASSERT_EQ(config.MaxPulseTimeNS, 5 * 71'428'571);
}

TEST_F(ConfigTest, NoConfigFile) {
  ASSERT_THROW(config = Config(""), std::runtime_error);
}

TEST_F(ConfigTest, JsonFileNotExist) {
  ASSERT_THROW(config = Config("/this_file_doesnt_exist"), std::runtime_error);
}

TEST_F(ConfigTest, NotJson) { ASSERT_ANY_THROW(config = Config(NotJsonFile)); }

TEST_F(ConfigTest, InvalidConfig) {
  ASSERT_ANY_THROW(config = Config(InvalidConfigFile));
}
//
// TEST_F(ConfigTest, ValidConfig) {
//   config = Config(ValidConfigFile);
//   ASSERT_EQ(config.getMaxPixel(), (32 + 24) * 4 * 7 * 256);
//   ASSERT_EQ(config.NTubesTotal, (32 + 24) * 4);
//   ASSERT_EQ(config.Panels.size(), 2);
//   deleteFile(ValidConfigFile);
// }

int main(int argc, char **argv) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  saveBuffer(InvalidConfigFile, (void *)InvalidConfigStr.c_str(),
             InvalidConfigStr.size());

  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  deleteFile(NotJsonFile);
  deleteFile(InvalidConfigFile);

  return RetVal;
}
