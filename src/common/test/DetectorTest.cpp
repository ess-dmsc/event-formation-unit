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
#include <memory>

class TestDetector : public Detector {
public:
  explicit TestDetector(BaseSettings settings) : Detector(settings){};
  ~TestDetector(){};
};

// Test fixture and tests below

class DetectorTest : public TestBase {
protected:
  BaseSettings settings;
  void SetUp() override {
    det = std::shared_ptr<Detector>(new Detector(settings));
  }

  void TearDown() override {}

  std::shared_ptr<Detector> det;
  void *dummyargs; // Used for calling thread functions
};

TEST_F(DetectorTest, Factory) { ASSERT_TRUE(det != nullptr); }

TEST_F(DetectorTest, StatAPI) {
  settings.DetectorName = "no detector";
  det = std::shared_ptr<Detector>(new Detector(settings));
  int res = det->statsize();
  // Detector initialize 35 statistics counters for input thread coutners and
  // for ESS header parser
  ASSERT_EQ(res, 35);

  int64_t val = det->statvalue(36);
  ASSERT_EQ(val, -1);
  val = det->statvalue(0);
  ASSERT_EQ(val, -1);

  auto detectorname = det->EFUSettings.DetectorName.c_str();
  ASSERT_STREQ("no detector", detectorname);
}

TEST_F(DetectorTest, ThreadInfoNoThreads) {
  auto &threadlist = det->GetThreadInfo();
  ASSERT_EQ(0, threadlist.size());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
