// Copyright (C) 2023 - 2025 European Spallation Source, see LICENSE file, ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Bifrost configuration parser
///
//===----------------------------------------------------------------------===//
#include <bifrost/geometry/BifrostConfig.h>
#include <common/testutils/TestBase.h>
#include <string>

// Valid JSON configuration
auto ValidJson = R"(
    {
      "Detector": "bifrost",
      "StrawResolution": 300,
      "MaxAmpl": 32767,
      "MaxRing": 4,
      "MaxGroup": 15
    }
  )"_json;

// Invalid detector name
auto InvalidDetectorJson = R"(
    {
      "Detector": "invalid",
      "StrawResolution": 300,
      "MaxAmpl": 32767
    }
  )"_json;

// Missing required field
auto MissingFieldJson = R"(
    {
      "Detector": "bifrost",
      "MaxAmpl": 32767
    }
  )"_json;

// Malformed JSON
const std::string MalformedJson = R"(
    {
      "Detector": "bifrost"
      "StrawResolution": 300,
      "MaxAmpl": 32767
    }
  )";

using namespace Caen;

class BifrostConfigTest : public TestBase {
protected:
  BifrostConfig Config;

  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(BifrostConfigTest, ParseValidConfig) {
  Config.setRoot(ValidJson);
  ASSERT_NO_THROW(Config.parseConfig());
  ASSERT_EQ(Config.Parms.Resolution, 300);
  ASSERT_EQ(Config.Parms.MaxAmpl, 32767);
}

TEST_F(BifrostConfigTest, InvalidDetectorName) {
  Config.setRoot(InvalidDetectorJson);
  ASSERT_THROW(Config.parseConfig(), std::runtime_error);
}

TEST_F(BifrostConfigTest, MissingRequiredField) {
  Config.setRoot(MissingFieldJson);
  ASSERT_THROW(Config.parseConfig(), std::runtime_error);
}

TEST_F(BifrostConfigTest, MalformedJson) {
  Config.setRoot(MalformedJson);
  ASSERT_THROW(Config.parseConfig(), std::exception);
}

TEST_F(BifrostConfigTest, DefaultConstructor) {
  ASSERT_EQ(Config.Parms.Resolution, 0);
  ASSERT_EQ(Config.Parms.MaxAmpl, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
