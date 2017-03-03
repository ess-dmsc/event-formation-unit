/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Detector.h>
#include <memory>
#include <test/TestBase.h>

#define UNUSED __attribute__((unused))

using namespace std;

class TestDetector : public Detector {
public:
  TestDetector(UNUSED void *args) { cout << "TestDetector" << endl; };
  ~TestDetector() { cout << "~TestDetector" << endl; };
};

class TestDetectorFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *args) {
    cout << "TestDetectorFactory" << endl;
    return std::shared_ptr<Detector>(new TestDetector(args));
  }
};

TestDetectorFactory Factory;

/** Test fixture and tests below */

class DetectorTest : public TestBase {
protected:
  virtual void SetUp() { det = Factory.create(0); }
  virtual void TearDown() {}
  std::shared_ptr<Detector> det;
  void *dummyargs; // Used for calling thread functions
};

TEST_F(DetectorTest, Destructor) {
  std::string output;
  testing::internal::CaptureStdout();
  {
    std::shared_ptr<Detector> tmp = Factory.create(0);
    output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, "TestDetectorFactory\nTestDetector\n");

    testing::internal::CaptureStdout();
  }

  // delete tmp;
  output = testing::internal::GetCapturedStdout();
  ASSERT_EQ(output, "~TestDetector\n");
}

TEST_F(DetectorTest, Factory) { ASSERT_TRUE(det != nullptr); }

TEST_F(DetectorTest, DefaultThreads) {
  testing::internal::CaptureStdout();
  det->input_thread(dummyargs);
  std::string output = testing::internal::GetCapturedStdout();
  ASSERT_EQ(output, "no input stage\n");

  testing::internal::CaptureStdout();
  det->processing_thread(dummyargs);
  output = testing::internal::GetCapturedStdout();
  ASSERT_EQ(output, "no processing stage\n");

  testing::internal::CaptureStdout();
  det->output_thread(dummyargs);
  output = testing::internal::GetCapturedStdout();
  ASSERT_EQ(output, "no output stage\n");
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
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
