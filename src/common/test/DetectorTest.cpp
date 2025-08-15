// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Detector class
///
//===----------------------------------------------------------------------===//

#include <common/detector/Detector.h>
#include <common/testutils/TestBase.h>
#include <gtest/gtest.h>
#include <memory>
#include <unordered_set>

/// TestDetector is a mock detector class for testing purposes
/// It inherits from Detector and can be used to create instances for testing
class TestDetector : public Detector {

public:
  /// \brief Constructor for the TestDetector class
  explicit TestDetector(BaseSettings settings) : Detector(settings) {}

  /// \brief Destructor for the TestDetector class
  ~TestDetector(){};
};

class DetectorTest : public TestBase {

protected:
  BaseSettings Settings;
  std::unique_ptr<Detector> DetectorPtr;

  std::unordered_set<std::string> ExpectedStatNames = {
      // Detector stats
      Detector::METRIC_RECEIVE_PACKETS, Detector::METRIC_RECEIVE_BYTES,
      Detector::METRIC_RECEIVE_DROPPED, Detector::METRIC_FIFO_SEQ_ERRORS,
      Detector::METRIC_THREAD_INPUT_IDLE,
      Detector::METRIC_TRANSMIT_CALIBMODE_PACKETS,

      // Parser stats
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_HEADER,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_BUFFER,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_COOKIE,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_PAD,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_SIZE,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_VERSION,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_OUTPUT_QUEUE,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_TYPE,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_SEQNO,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_TIMEHIGH,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_ERRORS_TIMEFRAC,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_HEARTBEATS,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_VERSION_V0,
      ESSReadout::Parser::METRIC_PARSER_ESSHEADER_VERSION_V1,

      // Time counter stats
      ESSReadout::Parser::METRIC_EVENTS_TIMESTAMP_TOF_COUNT,
      ESSReadout::Parser::METRIC_EVENTS_TIMESTAMP_TOF_NEGATIVE,
      ESSReadout::Parser::METRIC_EVENTS_TIMESTAMP_TOF_HIGH,
      ESSReadout::Parser::METRIC_EVENTS_TIMESTAMP_PREVTOF_COUNT,
      ESSReadout::Parser::METRIC_EVENTS_TIMESTAMP_PREVTOF_NEGATIVE,
      ESSReadout::Parser::METRIC_EVENTS_TIMESTAMP_PREVTOF_HIGH};

public:
  DetectorTest() {
    Settings.DetectorName = "no detector";
    // Ensure that we store stats without any prefix string
    // This is to avoid issues with the test environment
    Settings.GraphitePrefix = "";
    Settings.GraphiteRegion = "";

    DetectorPtr = std::make_unique<Detector>(Settings);

    // Add output queue stats (12 of them) - this still needs to be done
    // algorithmically
    for (int i = 0; i < 12; i++) {
      ExpectedStatNames.insert(fmt::format(
          ESSReadout::Parser::METRIC_PARSER_ESSHEADER_OQ_PACKETS, i));
    }
  }

  void SetUp() override {}

  void TearDown() override {}

  // Helper method to verify a single stat name exists in the expected list
  bool verifyStatName(const std::string &statName) {
    // Search for the stat name in our expected names list
    auto it =
        std::find(ExpectedStatNames.begin(), ExpectedStatNames.end(), statName);
    if (it != ExpectedStatNames.end()) {
      return true; // Found the stat name
    }
    // Not found
    std::cout << "  WARNING: Stat name not in expected list: " << statName
              << std::endl;
    return false;
  }
};

TEST_F(DetectorTest, TestConstructorAndStatCreation) {

  ASSERT_TRUE(DetectorPtr) << "Detector not created, unitialized pointer"
                           << std::endl;

  // Test with a name that shouldn't exist
  ASSERT_FALSE(verifyStatName("nonexistent.stat.name"));

  // Get the total number of stats
  int statSize = DetectorPtr->statsize();

  // Verify that the number of stats matches our expectation
  EXPECT_EQ(statSize, ExpectedStatNames.size());

  // Loop through all the detector stats and verify each one
  std::cout << "Verifying all registered stat names:" << std::endl;
  for (int i = 1; i < statSize; ++i) {
    auto statName = DetectorPtr->getStatFullName(i);

    // Assert that each stat name is in our expected list
    ASSERT_TRUE(verifyStatName(DetectorPtr->getStatFullName(i)))
        << "Stat name not in expected list: " << statName;
  }

  // Test invalid stat indices
  int64_t val = DetectorPtr->statvalue(ExpectedStatNames.size() + 1);
  ASSERT_EQ(val, -1);
  val = DetectorPtr->statvalue(0);
  ASSERT_EQ(val, -1);

  auto detectorname = DetectorPtr->EFUSettings.DetectorName.c_str();
  ASSERT_STREQ("no detector", detectorname);
}

TEST_F(DetectorTest, ThreadInfoNoThreads) {
  auto &threadlist = DetectorPtr->GetThreadInfo();
  ASSERT_EQ(0, threadlist.size());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
