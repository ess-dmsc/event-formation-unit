// Copyright (C) 2025 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for BEER configuration
///
//===----------------------------------------------------------------------===//

#include <beer/geometry/Config.h>
#include <common/testutils/TestBase.h>

#include <filesystem>

// clang-format off
auto WrongDetectorName = R"(
  {
    "Detector" : "CBM",
    "MaxFENId" : 1,

    "Topology" : [
      { "FEN":  0, "Channel": 0, "Source" : "beer1", "PixelOffset": 0, "Width": 512, "Height": 512 }
    ]
  }
)"_json;

auto CorrectBEERDetector = R"(
  {
    "Detector" : "BEER",
    "MaxFENId" : 1,

    "Topology" : [
      { "FEN":  0, "Channel": 0, "Source" : "beer1", "PixelOffset": 0, "Width": 512, "Height": 512 }
    ]
  }
)"_json;
// clang-format on

using namespace beer;

class BeerConfigTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(BeerConfigTest, Constructor) {
  Config BeerConfig("");
  
  // BEER Config should have BEER detector type set in constructor
  ASSERT_EQ(BeerConfig.Instrument, DetectorType::BEER);
}

TEST_F(BeerConfigTest, RejectsNonBEERDetectorName) {
  Config BeerConfig("");
  BeerConfig.root() = WrongDetectorName;

  // BEER Config should reject configs with "CBM" or other detector names
  try {
    BeerConfig.apply();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &err) {
    EXPECT_EQ(err.what(), std::string("Detector name mismatch, expected BEER"));
  }
}

TEST_F(BeerConfigTest, AcceptsBEERDetectorName) {
  Config BeerConfig("");
  BeerConfig.root() = CorrectBEERDetector;

  // BEER Config should accept configs with "BEER" as detector name
  ASSERT_NO_THROW(BeerConfig.apply());
  
  // Verify the detector type is correctly maintained
  ASSERT_EQ(BeerConfig.Instrument, DetectorType::BEER);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
