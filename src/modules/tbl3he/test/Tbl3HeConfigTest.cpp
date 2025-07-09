// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <tbl3he/geometry/Tbl3HeConfig.h>
#include <common/testutils/TestBase.h>
#include <common/testutils/SaveBuffer.h>

using namespace Caen;

std::string NotJsonFile{"deleteme_tbl3he_notjson.json"};
std::string NotJsonStr = R"(
{
  Ceci n'est pas Json
)";

// Valid Tbl3He configuration for testing - this is our base configuration
auto ValidTbl3HeConfigJSON = R"(
  {
    "Detector" : "tbl3he",
    "Resolution" : 2048,
    "NumOfFENs" : 4,
    "MinValidAmplitude" : 100,
    "Topology" : [
      { "Ring" : 0, "FEN" : 0, "Bank" : 0 },
      { "Ring" : 1, "FEN" : 1, "Bank" : 1 },
      { "Ring" : 2, "FEN" : 2, "Bank" : 2 },
      { "Ring" : 3, "FEN" : 3, "Bank" : 3 }
    ]
  }
)"_json;


class Tbl3HeConfigTest : public TestBase {
protected:
  Tbl3HeConfig config;
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(Tbl3HeConfigTest, Constructor) {
  ASSERT_EQ(config.Params.Resolution, 0);
  ASSERT_EQ(config.Params.MinValidAmplitude, 0);
  ASSERT_EQ(config.Params.NumOfFENs, 0);
  ASSERT_EQ(config.Params.MinRing, 0);
  ASSERT_EQ(config.Params.MaxRing, 11);
  ASSERT_EQ(config.TopologyMapPtr, nullptr);
}

TEST_F(Tbl3HeConfigTest, NoConfigFile) {
  ASSERT_THROW(Tbl3HeConfig MyConfig(""), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, JsonFileNotExist) {
  ASSERT_THROW(Tbl3HeConfig MyConfig("/this_file_doesnt_exist"), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, NotJson) {
  ASSERT_ANY_THROW(Tbl3HeConfig MyConfig(NotJsonFile));
  deleteFile(NotJsonFile);
}

TEST_F(Tbl3HeConfigTest, BadDetectorName) {
  auto badDetectorConfig = ValidTbl3HeConfigJSON;
  badDetectorConfig["Detector"] = "tbl3hexx";
  config.setRoot(badDetectorConfig);
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, MissingResolution) {
  auto missingResolutionConfig = ValidTbl3HeConfigJSON;
  missingResolutionConfig.erase("Resolution");
  config.setRoot(missingResolutionConfig);
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, ValidTbl3HeConfig) {
  config.setRoot(ValidTbl3HeConfigJSON);
  ASSERT_NO_THROW(config.parseConfig());
  
  // Check that Tbl3He config values from JSON are properly stored
  ASSERT_EQ(config.Params.Resolution, 2048);
  ASSERT_EQ(config.Params.NumOfFENs, 4);
  ASSERT_EQ(config.Params.MinValidAmplitude, 100);
  
  // Check topology configuration
  ASSERT_NE(config.TopologyMapPtr, nullptr);
  
  // Test specific topology mappings
  for (int i = 0; i < 4; i++) {
    ASSERT_TRUE(config.TopologyMapPtr->isValue(i, i));
    ASSERT_EQ(config.TopologyMapPtr->get(i, i)->Bank, i);
  }
}

TEST_F(Tbl3HeConfigTest, InvalidConfigMinRing) {
  auto invalidMinRingConfig = ValidTbl3HeConfigJSON;
  invalidMinRingConfig["Topology"][0]["Ring"] = -1; // outside interval 0 - 11
  config.setRoot(invalidMinRingConfig);
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, InvalidConfigMaxRing) {
  auto invalidMaxRingConfig = ValidTbl3HeConfigJSON;
  invalidMaxRingConfig["Topology"][0]["Ring"] = 12; // outside interval 0 - 11
  config.setRoot(invalidMaxRingConfig);
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, InvalidConfigNumOfFENs) {
  auto invalidNumOfFENsConfig = ValidTbl3HeConfigJSON;
  invalidNumOfFENsConfig["NumOfFENs"] = 5; // Doesn't match topology size
  config.setRoot(invalidNumOfFENsConfig);
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, InvalidConfigDuplicateRingFEN) {
  auto duplicateRingFENConfig = ValidTbl3HeConfigJSON;
  duplicateRingFENConfig["Topology"][1]["Ring"] = 0; // Duplicate of first topology Ring
  duplicateRingFENConfig["Topology"][1]["FEN"] = 0; // Duplicate of first topology FEN
  config.setRoot(duplicateRingFENConfig);
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, MissingTopology) {
  auto missingTopology = ValidTbl3HeConfigJSON;
  missingTopology.erase("Topology");
  config.setRoot(missingTopology);
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

TEST_F(Tbl3HeConfigTest, MissingMandatoryTopologyField) {
  auto badTopology = ValidTbl3HeConfigJSON;
  badTopology["Topology"][0].erase("Bank");
  config.setRoot(badTopology);
  ASSERT_THROW(config.parseConfig(), std::runtime_error);
}

int main(int argc, char **argv) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
