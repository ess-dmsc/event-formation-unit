// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <loki/geometry/LokiConfig.h>

using namespace Caen;

// Valid Loki configuration for testing - now with multiple banks and configs
auto ValidLokiConfig = R"(
  {
    "Detector" : "loki",

    "Resolution" : 512,
    "GroupsZ" : 4,

    "ReadoutConstDelayNS" : 0,
    "MaxPulseTimeNS" : 357142855,
    "MaxTOFNS" : 1000000000,

    "Banks" : [
       {"Bank" : 0, "ID" : "bank0", "GroupsN" : 56, "YOffset" : 0},
       {"Bank" : 1, "ID" : "bank1", "GroupsN" : 48, "YOffset" : 400},
       {"Bank" : 2, "ID" : "bank2", "GroupsN" : 32, "YOffset" : 800}
    ],

    "Config" : [
      { "Ring" : 0, "Bank" : 0, "FENs" : 16, "FENOffset" :  0},
      { "Ring" : 1, "Bank" : 1, "FENs" : 12, "FENOffset" :  0},
      { "Ring" : 2, "Bank" : 2, "FENs" : 8, "FENOffset" :  0}
    ]
  }
)"_json;

class LokiConfigTest : public TestBase {

public:
  LokiConfigTest() = default;
  ~LokiConfigTest() override = default;

protected:
  LokiConfig config{LOKI_CONFIG};
  LokiConfig emptyConfig; // For constructor/default value tests
  void SetUp() override {
    ASSERT_EQ(config.Parms.Resolution, 0); // check one var is uninitialised
  }
  void TearDown() override {}
};

TEST_F(LokiConfigTest, Constructor) {
  // Check that default values are correctly initialized
  ASSERT_EQ(emptyConfig.Parms.Resolution, 0);
  ASSERT_EQ(emptyConfig.Parms.ConfiguredBanks, 0);
  ASSERT_EQ(emptyConfig.Parms.ConfiguredRings, 0);
  ASSERT_EQ(emptyConfig.Parms.GroupsZ, 0);
  ASSERT_EQ(emptyConfig.Parms.TotalGroups, 0);

  // Check Bank and Ring default values
  for (int i = 0; i < emptyConfig.Parms.NumRings; i++) {
    ASSERT_EQ(emptyConfig.Parms.Rings[i].Bank, -1);
    ASSERT_EQ(emptyConfig.Parms.Rings[i].FENs, 0);
    ASSERT_EQ(emptyConfig.Parms.Rings[i].FENOffset, 0);
  }
  for (int i = 0; i < emptyConfig.Parms.NumBanks; i++) {
    ASSERT_EQ(emptyConfig.Parms.Banks[i].GroupsN, 0);
    ASSERT_EQ(emptyConfig.Parms.Banks[i].BankName, "");
    ASSERT_EQ(emptyConfig.Parms.Banks[i].YOffset, 0);
  }
}

TEST_F(LokiConfigTest, ParseConfig) {
  ASSERT_NO_THROW(config.parseConfig());
  ASSERT_EQ(config.Parms.Resolution, 512);
  ASSERT_EQ(config.Parms.ConfiguredBanks, 9);
  ASSERT_EQ(config.Parms.ConfiguredRings, 10);
}

TEST_F(LokiConfigTest, ValidLokiConfigTest) {
  LokiConfig testConfig;
  testConfig.setRoot(ValidLokiConfig);
  testConfig.parseConfig();

  // Check that Loki config values from JSON are properly stored
  ASSERT_EQ(testConfig.Parms.Resolution, 512);
  ASSERT_EQ(testConfig.Parms.GroupsZ, 4);

  // Check Banks configuration - now verifying multiple banks
  ASSERT_EQ(testConfig.Parms.ConfiguredBanks, 3);
  
  // Check bank 0
  ASSERT_EQ(testConfig.Parms.Banks[0].GroupsN, 56);
  ASSERT_EQ(testConfig.Parms.Banks[0].YOffset, 0);
  ASSERT_EQ(testConfig.Parms.Banks[0].BankName, "bank0");
  
  // Check bank 1
  ASSERT_EQ(testConfig.Parms.Banks[1].GroupsN, 48);
  ASSERT_EQ(testConfig.Parms.Banks[1].YOffset, 400);
  ASSERT_EQ(testConfig.Parms.Banks[1].BankName, "bank1");
  
  // Check bank 2
  ASSERT_EQ(testConfig.Parms.Banks[2].GroupsN, 32);
  ASSERT_EQ(testConfig.Parms.Banks[2].YOffset, 800);
  ASSERT_EQ(testConfig.Parms.Banks[2].BankName, "bank2");

  // Check Ring configuration
  ASSERT_EQ(testConfig.Parms.ConfiguredRings, 3);
  ASSERT_EQ(testConfig.Parms.Rings[0].Bank, 0);
  ASSERT_EQ(testConfig.Parms.Rings[0].FENs, 16);
  ASSERT_EQ(testConfig.Parms.Rings[0].FENOffset, 0);
  ASSERT_EQ(testConfig.Parms.Rings[1].Bank, 1);
  ASSERT_EQ(testConfig.Parms.Rings[1].FENs, 12);
  ASSERT_EQ(testConfig.Parms.Rings[1].FENOffset, 0);
  ASSERT_EQ(testConfig.Parms.Rings[2].Bank, 2);
  ASSERT_EQ(testConfig.Parms.Rings[2].FENs, 8);
  ASSERT_EQ(testConfig.Parms.Rings[2].FENOffset, 0);

  // Check total groups calculation is correct
  int expectedTotalGroups = 56 * 4 + 48 * 4 + 32 * 4; // Sum of (GroupsN * GroupsZ) for all banks
  ASSERT_EQ(testConfig.Parms.TotalGroups, expectedTotalGroups);
}

TEST_F(LokiConfigTest, InvalidConfigTest) {
  // Test missing Resolution field
  LokiConfig testConfig;
  auto invalidConfig = ValidLokiConfig;
  invalidConfig.erase("Resolution");
  testConfig.setRoot(invalidConfig);

  try {
    testConfig.parseConfig();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    EXPECT_STREQ("Invalid Json file", e.what());
  } catch (...) {
    FAIL() << "Expected std::runtime_error with 'Invalid Json file' message";
  }
}

TEST_F(LokiConfigTest, BadDetectorNameTest) {
  // Test invalid detector name
  LokiConfig testConfig;
  auto badNameConfig = ValidLokiConfig;
  badNameConfig["Detector"] = "lokixx";
  testConfig.setRoot(badNameConfig);

  try {
    testConfig.parseConfig();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    EXPECT_STREQ("InstrumentName != 'loki'", e.what());
  } catch (...) {
    FAIL() << "Expected std::runtime_error with 'InstrumentName != 'loki'' "
              "message";
  }
}

TEST_F(LokiConfigTest, NoDetectorKey) {
  // Test missing Detector field
  LokiConfig testConfig;
  auto noDetectorConfig = ValidLokiConfig;
  noDetectorConfig.erase("Detector");
  testConfig.setRoot(noDetectorConfig);

  try {
    testConfig.parseConfig();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    EXPECT_STREQ("Missing 'Detector' field", e.what());
  } catch (...) {
    FAIL() << "Expected std::runtime_error with 'Missing 'Detector' field' "
              "message";
  }
}

TEST_F(LokiConfigTest, MissingGroupsZTest) {
  // Test missing GroupsZ field - it should not be mandatory
  LokiConfig testConfig;
  auto missingGroupsZ = ValidLokiConfig;
  missingGroupsZ.erase("GroupsZ");
  testConfig.setRoot(missingGroupsZ);

  // Should not throw an exception
  ASSERT_NO_THROW(testConfig.parseConfig());

  // GroupsZ should remain at default value (0)
  ASSERT_EQ(testConfig.Parms.GroupsZ, 0);

  // Other fields should be correctly parsed
  ASSERT_EQ(testConfig.Parms.Resolution, 512);
  ASSERT_EQ(testConfig.Parms.ConfiguredBanks, 3);
  ASSERT_EQ(testConfig.Parms.ConfiguredRings, 3);
}

TEST_F(LokiConfigTest, MissingBanksTest) {
  // Test missing Banks field - it should not be mandatory
  LokiConfig testConfig;
  auto missingBanks = ValidLokiConfig;
  missingBanks.erase("Banks");
  testConfig.setRoot(missingBanks);

  // Should not throw an exception
  ASSERT_NO_THROW(testConfig.parseConfig());

  // ConfiguredBanks should remain at default value (0)
  ASSERT_EQ(testConfig.Parms.ConfiguredBanks, 0);
  ASSERT_EQ(testConfig.Parms.TotalGroups, 0); // No banks, so no total groups
}

TEST_F(LokiConfigTest, MissingConfigTest) {
  // Test missing Config field
  LokiConfig testConfig;
  auto missingConfig = ValidLokiConfig;
  missingConfig.erase("Config");
  testConfig.setRoot(missingConfig);

  // Should not throw an exception
  ASSERT_NO_THROW(testConfig.parseConfig());

  ASSERT_EQ(testConfig.Parms.ConfiguredRings, 0); // No config, so no rings
}

TEST_F(LokiConfigTest, BadBank) {
  config["Banks"][0]["Bank"] = 200;
  ASSERT_NO_THROW(config.parseConfig());
  ASSERT_EQ(config.Parms.ConfiguredBanks, 8);
}

TEST_F(LokiConfigTest, BadRing) {
  config["Config"][0]["Ring"] = 200;
  ASSERT_NO_THROW(config.parseConfig());
  ASSERT_EQ(config.Parms.ConfiguredRings, 9);
}

TEST_F(LokiConfigTest, GetGlobalGroup) {
  config.parseConfig();
  //  Ring, FEN, (Local)Group     R   F  LG
  // Validating partitioned bank0
  ASSERT_EQ(config.getGlobalGroup(0, 0, 0), 0);
  ASSERT_EQ(config.getGlobalGroup(0, 0, 4), 1);
  ASSERT_EQ(config.getGlobalGroup(0, 15, 4), 31);
  ASSERT_EQ(config.getGlobalGroup(1, 0, 0), 32);
  ASSERT_EQ(config.getGlobalGroup(1, 0, 4), 33);
  ASSERT_EQ(config.getGlobalGroup(1, 11, 4), 55);
  ASSERT_EQ(config.getGlobalGroup(1, 11, 7), 223);

  ASSERT_EQ(config.getGlobalGroup(0, 0, 1), 0 + 56);
  ASSERT_EQ(config.getGlobalGroup(0, 15, 5), 31 + 56);
  ASSERT_EQ(config.getGlobalGroup(1, 0, 1), 32 + 56);
  ASSERT_EQ(config.getGlobalGroup(1, 11, 5), 55 + 56);

  // Ad hoc checking
  ASSERT_EQ(config.getGlobalGroup(2, 0, 0), 224);
  ASSERT_EQ(config.getGlobalGroup(2, 0, 1), 224 + 16);

  // Last Group
  ASSERT_EQ(config.getGlobalGroup(9, 15, 7), 895);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
