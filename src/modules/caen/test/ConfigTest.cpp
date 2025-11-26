// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
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
  Ceci n'est pas Json
)";

// Base configuration for all tests
auto BaseConfigJSON = R"(
  {
    "Detector" : "loki",
    "Resolution" : 1024,
    "GroupsZ" : 4,
    "MaxAmpl" : 65535,
    "NumOfFENs" : 2,
    "MinValidAmplitude" : 100,
    "ReadoutConstDelayNS" : 0,
    "MaxPulseTimeNS" : 357142855,
    "MaxTOFNS" : 1000000000,
    "MaxFEN" : 16,
    "MaxGroup" : 32,

    "Banks" : [
       {"Bank" : 0, "ID" : "bank0", "GroupsN" : 56, "YOffset" : 0}
    ],

    "Config" : [
      { "Ring" : 0, "Bank" : 0, "FENs" : 16, "FENOffset" :  0}
    ],
    
    "Topology" : [
      { "Ring" : 0, "FEN" : 0, "Bank" : 0 },
      { "Ring" : 1, "FEN" : 1, "Bank" : 1 }
    ]
  }
)"_json;

using namespace Caen;

class CaenConfigTest : public TestBase {
protected:
  Config config;
  nlohmann::json testConfig;

  void SetUp() override {
    // Reset to base config before each test
    testConfig = BaseConfigJSON;
  }

  void TearDown() override {}

  // Helper method to modify detector type
  void setDetector(const std::string &detector) {
    testConfig["Detector"] = detector;
  }

  // Helper method to remove a field from the config
  void removeField(const std::string &field) { testConfig.erase(field); }
};

TEST_F(CaenConfigTest, Constructor) {
  // Test CaenParms default values
  ASSERT_EQ(config.CaenParms.InstrumentName, "");
  ASSERT_EQ(config.CaenParms.MaxGroup, 0);
  ASSERT_EQ(config.CaenParms.MaxPulseTimeNS, 5 * 71'428'571);
  ASSERT_EQ(config.CaenParms.MaxTOFNS, 20 * 71'428'571);
  ASSERT_EQ(config.CaenParms.MaxFEN, 0);
  ASSERT_EQ(config.CaenParms.MinRing, 0);
  ASSERT_EQ(config.CaenParms.MaxRing, 11);

  // Test LokiConf default values
  ASSERT_EQ(config.LokiConf.Parms.ConfiguredBanks, 0);
  ASSERT_EQ(config.LokiConf.Parms.ConfiguredRings, 0);
  ASSERT_EQ(config.LokiConf.Parms.GroupsZ, 0);
  ASSERT_EQ(config.LokiConf.Parms.TotalGroups, 0);

  // Test Tbl3HeConf default values
  ASSERT_EQ(config.Tbl3HeConf.Tbl3HeParms.NumOfFENs, 0);
  ASSERT_EQ(config.Tbl3HeConf.Tbl3HeParms.MinValidAmplitude, 0);
  ASSERT_EQ(config.Tbl3HeConf.Tbl3HeParms.MinRing, 0);
  ASSERT_EQ(config.Tbl3HeConf.Tbl3HeParms.MaxRing, 11);
  ASSERT_EQ(config.Tbl3HeConf.TopologyMapPtr, nullptr);

  // Test BifrostConf default values
  ASSERT_EQ(config.BifrostConf.Parms.MaxAmpl, std::numeric_limits<int>::max());
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
  // Test with invalid detector name
  setDetector("lokixx");
  config.setRoot(testConfig);
  ASSERT_ANY_THROW(config.parseConfig());
}

TEST_F(CaenConfigTest, InvalidConfig) {
  // Test with missing Detector field
  removeField("Detector");
  testConfig["NotDetector"] = "LoKI4x8"; // Invalid field name
  config.setRoot(testConfig);
  ASSERT_ANY_THROW(config.parseConfig());
}

TEST_F(CaenConfigTest, MissingMaxFEN) {
  removeField("MaxFEN");
  config.setRoot(testConfig);

  // Check that default value is applied
  ASSERT_EQ(config.CaenParms.MaxFEN, 0); // Default value
}

TEST_F(CaenConfigTest, MissingMaxGroup) {
  removeField("MaxGroup");
  config.setRoot(testConfig);

  // Check that default value is applied
  ASSERT_EQ(config.CaenParms.MaxGroup, 0); // Default value
}

/// \brief Test that the loki identifed and related loki config is parsed
/// correctly
TEST_F(CaenConfigTest, ValidLokiConfig) {
  config.setRoot(testConfig);
  config.parseConfig();

  // Check total groups calculation is correct
  ASSERT_EQ(config.LokiConf.Parms.TotalGroups, (32 + 24) * 4);

  // Check that Loki config values are set correctly
  ASSERT_EQ(config.LokiConf.Parms.GroupsZ, 4);

  // Check CAEN general parameters
  ASSERT_EQ(config.CaenParms.MaxPulseTimeNS, 357142855);
  ASSERT_EQ(config.CaenParms.MaxTOFNS, 1000000000);
  ASSERT_EQ(config.CaenParms.InstrumentName, "loki");
}

/// \brief Test that the bifrost identifed and related bifrost config is parsed
/// correctly
TEST_F(CaenConfigTest, ValidBifrostConfig) {
  setDetector("bifrost");
  config.setRoot(testConfig);
  config.parseConfig();

  // Check that Bifrost some specific values are set correctly
  ASSERT_EQ(config.BifrostConf.Parms.MaxAmpl, 65535);

  // Check CAEN general parameters
  ASSERT_EQ(config.CaenParms.MaxPulseTimeNS, 357142855);
  ASSERT_EQ(config.CaenParms.MaxTOFNS, 1000000000);
  ASSERT_EQ(config.CaenParms.InstrumentName, "bifrost");
  ASSERT_EQ(config.CaenParms.Resolution, 1024);
}

/// \brief Test that the tbl3he identifed and related tbl3he config is parsed
/// correctly
TEST_F(CaenConfigTest, ValidTbl3HeConfig) {
  setDetector("tbl3he");
  config.setRoot(testConfig);
  config.parseConfig();

  // Verify Tbl3He specific parameters are set correctly
  ASSERT_EQ(config.Tbl3HeConf.Tbl3HeParms.NumOfFENs, 2);
  ASSERT_EQ(config.Tbl3HeConf.Tbl3HeParms.MinValidAmplitude, 100);

  // Check CAEN general parameters
  ASSERT_EQ(config.CaenParms.MaxPulseTimeNS, 357142855);
  ASSERT_EQ(config.CaenParms.MaxTOFNS, 1000000000);
  ASSERT_EQ(config.CaenParms.InstrumentName, "tbl3he");
}

TEST_F(CaenConfigTest, MissingResolution) {
  // Test missing Resolution field
  auto invalidConfig = BaseConfigJSON;
  invalidConfig.erase("Resolution");
  config.setRoot(invalidConfig);

  try {
    config.parseConfig();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    EXPECT_STREQ("Invalid Json file", e.what());
  } catch (...) {
    FAIL() << "Expected std::runtime_error with 'Invalid Json file' message";
  }
}

/// \brief Test default MaxTOFNS value when not specified in config
TEST_F(CaenConfigTest, DefaultMaxTofConfig) {

  const uint64_t DefaultMaxTofNS = 20 * 71'428'571; // 20 * 14 Hz pulse period

  removeField("MaxTOFNS");
  config.setRoot(testConfig);
  config.parseConfig();

  ASSERT_EQ(config.CaenParms.MaxTOFNS, DefaultMaxTofNS);
}

int main(int argc, char **argv) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}