/** Copyright (C) 2016 European Spallation Source ERIC */

#include "TestBase.h"
#include <common/Detector.h>
#include <memory>

using namespace std;

class TestDetector : public Detector {
public:
  TestDetector() { cout << "TestDetector" << endl; };
  ~TestDetector() { cout << "~TestDetector" << endl; };
};

class TestDetectorFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create() {
    cout << "TestDetectorFactory" << endl;
    return std::shared_ptr<Detector> (new TestDetector);
  }
};

TestDetectorFactory Factory;

/** Test fixture and tests below */

class DetectorTest : public TestBase {
protected:
  virtual void SetUp() { det = Factory.create(); }
  virtual void TearDown() { }
  std::shared_ptr<Detector> det;
  void * dummyargs; // Used for calling thread functions
};

TEST_F(DetectorTest, Destructor) {
  std::string output;
  testing::internal::CaptureStdout();
  {
    std::shared_ptr<Detector> tmp = Factory.create();
    output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, "TestDetectorFactory\nTestDetector\n");

    testing::internal::CaptureStdout();
  }

  //delete tmp;
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

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
