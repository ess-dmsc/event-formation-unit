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

public:
  DetectorTest() {
    Settings.DetectorName = "no detector";
    // Ensure that we store stats without any prefix string
    // This is to avoid issues with the test environment
    Settings.GraphitePrefix = "";
    Settings.GraphiteRegion = "";

    DetectorPtr = std::make_unique<Detector>(Settings);
  }

  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(DetectorTest, TestConstructorAndStatCreation) {

  ASSERT_TRUE(DetectorPtr) << "Detector not created, unitialized pointer"
                           << std::endl;

  // Get the total number of stats
  int statSize = DetectorPtr->statsize();
  // Update this in case of new counters introduced for detector class
  constexpr int ExpectedStatCount = 71;
  EXPECT_EQ(statSize, ExpectedStatCount);

  // Test invalid stat indices
  int64_t val = DetectorPtr->getStatValue(statSize + 1);
  ASSERT_EQ(val, -1);
  val = DetectorPtr->getStatValue(0);
  ASSERT_EQ(val, -1);

  auto detectorname = DetectorPtr->getDetectorName().c_str();
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
