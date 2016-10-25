/** Copyright (C) 2016 European Spallation Source */

#include "TestBase.h"
#include <common/Detector.h>

using namespace std;

class TestDetector : public Detector {
public:
  TestDetector() { cout << "    TestDetector created" << endl; };
  ~TestDetector() { cout << "    TestDetector destroyed" << endl; };
};

class TestDetectorFactory : public DetectorFactory {
public:
  Detector *create() {
    cout << "    making TestDetector" << endl;
    return new TestDetector;
  }
};

TestDetectorFactory Factory;

/** Test fixture and tests below */

class DetectorTest : public TestBase {
protected:
  virtual void SetUp() { det = Factory.create(); }
  Detector *det;
};

TEST_F(DetectorTest, Factory) { ASSERT_TRUE(det != NULL); }

TEST_F(DetectorTest, DefaultThreads) {
  testing::internal::CaptureStdout();
  det->input_thread(det);
  std::string output = testing::internal::GetCapturedStdout();
  ASSERT_EQ(output, "no input stage\n");

  testing::internal::CaptureStdout();
  det->processing_thread(det);
  output = testing::internal::GetCapturedStdout();
  ASSERT_EQ(output, "no processing stage\n");

  testing::internal::CaptureStdout();
  det->output_thread(det);
  output = testing::internal::GetCapturedStdout();
  ASSERT_EQ(output, "no output stage\n");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
