/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/detector/Detector.h>
#include <common/testutils/TestBase.h>
#include <memory>

#define UNUSED __attribute__((unused))

class TestDetector : public Detector {
public:
  explicit TestDetector(BaseSettings settings) : Detector(settings) {
    std::cout << "TestDetector" << std::endl;
  };
  ~TestDetector() { std::cout << "~TestDetector" << std::endl; };
};

DetectorFactory<TestDetector> Factory;

/** Test fixture and tests below */

class DetectorTest : public TestBase {
protected:
  BaseSettings settings;
  void SetUp() override { det = Factory.create(settings); }

  void TearDown() override {}

  std::shared_ptr<Detector> det;
  void *dummyargs; // Used for calling thread functions
};

TEST_F(DetectorTest, Factory) { ASSERT_TRUE(det != nullptr); }

TEST_F(DetectorTest, StatAPI) {
  settings.DetectorName = "no detector";
  det = Factory.create(settings);
  int res = det->statsize();
  ASSERT_EQ(res, 0);

  int64_t val = det->statvalue(1);
  ASSERT_EQ(val, -1);
  val = det->statvalue(0);
  ASSERT_EQ(val, -1);

  auto name = det->statname(1);
  ASSERT_EQ("", name);

  auto detectorname = det->EFUSettings.DetectorName.c_str();
  ASSERT_STREQ("no detector", detectorname);
}

TEST_F(DetectorTest, ThreadInfoNoThreads) {
  auto &threadlist = det->GetThreadInfo();
  ASSERT_EQ(0, threadlist.size());
}

TEST_F(DetectorTest, GetDetectorCommandFunctionsNoCommands) {
  auto commandmap = det->GetDetectorCommandFunctions();
  ASSERT_EQ(0, commandmap.size());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
