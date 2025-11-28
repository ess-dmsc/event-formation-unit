// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for Timepix3 Config
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <timepix3/geometry/Config.h>

using namespace Timepix3;

// Test helper class that exposes protected methods for testing
class TestableConfig : public Config {
public:
  TestableConfig() : Config() {}
  
  void parseFromJson(const nlohmann::json &json) {
    setRoot(json);
    
    // Manually parse the config like the constructor does
    // First set CHECK flag for required fields
    setMask(Flags::LOG | Flags::CHECK);
    assign("Detector", InstrumentName);
    if (InstrumentName != "timepix3") {
      throw std::runtime_error(
          "Inconsistent Json file - invalid name, expected timepix3");
    }
    assign("XResolution", XResolution);
    assign("YResolution", YResolution);
    assign("ParallelThreads", ParallelThreads);
    
    // Optional parameters (without CHECK flag)
    setMask(Flags::LOG);
    assign("ScaleUpFactor", ScaleUpFactor);
    assign("MaxTimeGapNS", MaxTimeGapNS);
    assign("MinEventTimeSpanNS", MinEventTimeSpanNS);
    assign("FrequencyHz", FrequencyHz);
    assign("MinEventSizeHits", MinEventSizeHits);
    assign("MinimumToTSum", MinimumToTSum);
    assign("MaxCoordinateGap", MaxCoordinateGap);
  }
};

std::string NotJsonFile{"deleteme_timepix3_notjson.json"};
std::string NotJsonStr = R"(
{
  Ceci n'est pas Json
)";

// Base configuration for all tests
auto BaseConfigJSON = R"(
  {
    "Detector": "timepix3",
    "XResolution": 256,
    "YResolution": 256,
    "ParallelThreads": 1
  }
)"_json;

class Timepix3ConfigTest : public TestBase {
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
  void removeField(const std::string &field) { 
    testConfig.erase(field); 
  }

  // Helper method to set a field value
  template<typename T>
  void setField(const std::string &field, const T &value) {
    testConfig[field] = value;
  }

  // Helper to create a Config from a json object
  Config createConfigFromJson(const nlohmann::json &json) {
    TestableConfig tempConfig;
    tempConfig.parseFromJson(json);
    return tempConfig;
  }
};

TEST_F(Timepix3ConfigTest, Constructor) {
  // Test default values
  ASSERT_EQ(config.InstrumentName, "");
  ASSERT_EQ(config.XResolution, 0);
  ASSERT_EQ(config.YResolution, 0);
  ASSERT_EQ(config.ScaleUpFactor, 1);
  ASSERT_EQ(config.ParallelThreads, 1);
  ASSERT_EQ(config.FrequencyHz, 14.0f);
  ASSERT_EQ(config.MaxTimeGapNS, 1);
  ASSERT_EQ(config.MinEventSizeHits, 1);
  ASSERT_EQ(config.MinimumToTSum, 20);
  ASSERT_EQ(config.MinEventTimeSpanNS, 1);
  ASSERT_EQ(config.MaxCoordinateGap, 5);
}

TEST_F(Timepix3ConfigTest, NoConfigFile) {
  ASSERT_THROW(config = Config(""), std::runtime_error);
}

TEST_F(Timepix3ConfigTest, JsonFileNotExist) {
  ASSERT_THROW(config = Config("/this_file_doesnt_exist"), std::runtime_error);
}

TEST_F(Timepix3ConfigTest, NotJson) {
  ASSERT_ANY_THROW(config = Config(NotJsonFile));
  deleteFile(NotJsonFile);
}

TEST_F(Timepix3ConfigTest, BadDetectorName) {
  // Test with invalid detector name
  setDetector("timepix4");
  ASSERT_ANY_THROW(createConfigFromJson(testConfig));
}

TEST_F(Timepix3ConfigTest, MissingDetectorField) {
  // Test with missing Detector field (required with CHECK flag)
  removeField("Detector");
  ASSERT_THROW(createConfigFromJson(testConfig), std::runtime_error);
}

TEST_F(Timepix3ConfigTest, MissingXResolution) {
  // XResolution is required (CHECK flag is set)
  removeField("XResolution");
  ASSERT_THROW(createConfigFromJson(testConfig), std::runtime_error);
}

TEST_F(Timepix3ConfigTest, MissingYResolution) {
  // YResolution is required (CHECK flag is set)
  removeField("YResolution");
  ASSERT_THROW(createConfigFromJson(testConfig), std::runtime_error);
}

TEST_F(Timepix3ConfigTest, MissingParallelThreads) {
  // ParallelThreads is required (CHECK flag is set)
  removeField("ParallelThreads");
  ASSERT_THROW(createConfigFromJson(testConfig), std::runtime_error);
}

TEST_F(Timepix3ConfigTest, OptionalScaleUpFactor) {
  // ScaleUpFactor is optional, should use default value
  removeField("ScaleUpFactor");
  Config tempConfig = createConfigFromJson(testConfig);
  
  ASSERT_EQ(tempConfig.ScaleUpFactor, 1); // Default value
}

TEST_F(Timepix3ConfigTest, OptionalMaxTimeGapNS) {
  // MaxTimeGapNS is optional, should use default value
  removeField("MaxTimeGapNS");
  Config tempConfig = createConfigFromJson(testConfig);
  
  ASSERT_EQ(tempConfig.MaxTimeGapNS, 1); // Default value
}

TEST_F(Timepix3ConfigTest, OptionalMinEventTimeSpanNS) {
  // MinEventTimeSpanNS is optional, should use default value
  removeField("MinEventTimeSpanNS");
  Config tempConfig = createConfigFromJson(testConfig);
  
  ASSERT_EQ(tempConfig.MinEventTimeSpanNS, 1); // Default value
}

TEST_F(Timepix3ConfigTest, OptionalFrequencyHz) {
  // FrequencyHz is optional, should use default value
  removeField("FrequencyHz");
  Config tempConfig = createConfigFromJson(testConfig);
  
  ASSERT_EQ(tempConfig.FrequencyHz, 14.0f); // Default value
}

TEST_F(Timepix3ConfigTest, OptionalMinEventSizeHits) {
  // MinEventSizeHits is optional, should use default value
  removeField("MinEventSizeHits");
  Config tempConfig = createConfigFromJson(testConfig);
  
  ASSERT_EQ(tempConfig.MinEventSizeHits, 1); // Default value
}

TEST_F(Timepix3ConfigTest, OptionalMinimumToTSum) {
  // MinimumToTSum is optional, should use default value
  removeField("MinimumToTSum");
  Config tempConfig = createConfigFromJson(testConfig);
  
  ASSERT_EQ(tempConfig.MinimumToTSum, 20); // Default value
}

TEST_F(Timepix3ConfigTest, OptionalMaxCoordinateGap) {
  // MaxCoordinateGap is optional, should use default value
  removeField("MaxCoordinateGap");
  Config tempConfig = createConfigFromJson(testConfig);
  
  ASSERT_EQ(tempConfig.MaxCoordinateGap, 5); // Default value
}

TEST_F(Timepix3ConfigTest, ValidConfigWithAllFields) {
  // Set all fields to custom values
  setField("ScaleUpFactor", 4);
  setField("MaxTimeGapNS", 800);
  setField("MaxCoordinateGap", 4);
  setField("MinEventSizeHits", 9);
  setField("FrequencyHz", 10.0);
  setField("MinimumToTSum", 30);
  setField("MinEventTimeSpanNS", 500);

  Config tempConfig = createConfigFromJson(testConfig);

  // Verify required fields
  ASSERT_EQ(tempConfig.InstrumentName, "timepix3");
  ASSERT_EQ(tempConfig.XResolution, 256);
  ASSERT_EQ(tempConfig.YResolution, 256);
  ASSERT_EQ(tempConfig.ParallelThreads, 1);

  // Verify optional fields with custom values
  ASSERT_EQ(tempConfig.ScaleUpFactor, 4);
  ASSERT_EQ(tempConfig.MaxTimeGapNS, 800);
  ASSERT_EQ(tempConfig.MaxCoordinateGap, 4);
  ASSERT_EQ(tempConfig.MinEventSizeHits, 9);
  ASSERT_FLOAT_EQ(tempConfig.FrequencyHz, 10.0f);
  ASSERT_EQ(tempConfig.MinimumToTSum, 30);
  ASSERT_EQ(tempConfig.MinEventTimeSpanNS, 500);
}

TEST_F(Timepix3ConfigTest, BoundaryValuesXResolution) {
  // Test with valid boundary values for XResolution
  setField("XResolution", 1);
  Config tempConfig = createConfigFromJson(testConfig);
  ASSERT_EQ(tempConfig.XResolution, 1);

  setField("XResolution", 65535);
  Config tempConfig2 = createConfigFromJson(testConfig);
  ASSERT_EQ(tempConfig2.XResolution, 65535);

  // Test beyond uint16_t max (65535) - wraps to 0
  setField("XResolution", 65536);
  Config tempConfig3 = createConfigFromJson(testConfig);
  ASSERT_EQ(tempConfig3.XResolution, 0);

  // Test with negative value - wraps to max value
  setField("XResolution", -1);
  Config tempConfig4 = createConfigFromJson(testConfig);
  ASSERT_EQ(tempConfig4.XResolution, 65535);

  // Test with zero - parser accepts but logically invalid
  setField("XResolution", 0);
  Config tempConfig5 = createConfigFromJson(testConfig);
  ASSERT_EQ(tempConfig5.XResolution, 0);
}

TEST_F(Timepix3ConfigTest, BoundaryValuesYResolution) {
  // Test with valid boundary values for YResolution
  setField("YResolution", 1);
  Config tempConfig = createConfigFromJson(testConfig);
  ASSERT_EQ(tempConfig.YResolution, 1);

  setField("YResolution", 65535);
  Config tempConfig2 = createConfigFromJson(testConfig);
  ASSERT_EQ(tempConfig2.YResolution, 65535);

  // Test beyond uint16_t max (65535) - wraps to 0
  setField("YResolution", 65536);
  Config tempConfig3 = createConfigFromJson(testConfig);
  ASSERT_EQ(tempConfig3.YResolution, 0);

  // Test with negative value - wraps to max value
  setField("YResolution", -1);
  Config tempConfig4 = createConfigFromJson(testConfig);
  ASSERT_EQ(tempConfig4.YResolution, 65535);

  // Test with zero - parser accepts but logically invalid
  setField("YResolution", 0);
  Config tempConfig5 = createConfigFromJson(testConfig);
  ASSERT_EQ(tempConfig5.YResolution, 0);
}

int main(int argc, char **argv) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
