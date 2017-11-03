/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Detector.h>
#include <memory>
#include <test/TestBase.h>

#define UNUSED __attribute__((unused))

class TestDetector : public Detector {
public:
  TestDetector(UNUSED void *args) { std::cout << "TestDetector" << std::endl; };
  ~TestDetector() { std::cout << "~TestDetector" << std::endl; };
};

class TestDetectorFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *args) {
    std::cout << "TestDetectorFactory" << std::endl;
    return std::shared_ptr<Detector>(new TestDetector(args));
  }
};

TestDetectorFactory Factory;

/** Test fixture and tests below */

class DetectorTest : public TestBase {
protected:
  virtual void SetUp() { det = Factory.create(0); }

  virtual void TearDown() {}

  void assertcapture(const char * expected_output) {
    auto captured_output = testing::internal::GetCapturedStdout();
    ASSERT_TRUE(captured_output.find(expected_output) !=  std::string::npos);
  };

  std::shared_ptr<Detector> det;
  void *dummyargs; // Used for calling thread functions
};

TEST_F(DetectorTest, Destructor) {
  std::string output;
  testing::internal::CaptureStdout();
  {
    std::shared_ptr<Detector> tmp = Factory.create(0);
    assertcapture("TestDetectorFactory\nTestDetector\n");

    testing::internal::CaptureStdout();
  }

  // delete tmp;
  assertcapture("~TestDetector\nVirtual detector destructor called\n");
}

TEST_F(DetectorTest, Factory) { ASSERT_TRUE(det != nullptr); }

TEST_F(DetectorTest, DefaultThreads) {
  testing::internal::CaptureStdout();
  det->input_thread();
  assertcapture("no input stage");

  testing::internal::CaptureStdout();
  det->processing_thread();
  assertcapture("no processing stage");

  testing::internal::CaptureStdout();
  det->output_thread();
  assertcapture("no output stage");
}

TEST_F(DetectorTest, StatAPI) {
  int res = det->statsize();
  ASSERT_EQ(res, 0);

  int64_t val = det->statvalue(1);
  ASSERT_EQ(val, -1);
  val = det->statvalue(0);
  ASSERT_EQ(val, -1);

  auto name = det->statname(1);
  ASSERT_EQ("", name);

  auto detectorname = det->detectorname();
  ASSERT_EQ("no detector", detectorname);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
