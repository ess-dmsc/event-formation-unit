/** Copyright (C) 2018 European Spallation Source ERIC */

#include <gdgem/vmm3/CalibrationFile.h>
#include <gdgem/vmm3/CalibrationFileTestData.h>
#include <prototype2/common/DataSave.h>
#include <test/TestBase.h>
#include <vector>

class CalibrationFileTest : public TestBase {
protected:
  // virtual void SetUp() {  }
  // virtual void TearDown() {  }
};

/** Test cases below */
TEST_F(CalibrationFileTest, Constructor) {
  CalibrationFile cf;
  for (int fec = 0; fec < CalibrationFile::MAX_FEC; fec++) {
    for (int vmm = 0; vmm < CalibrationFile::MAX_VMM; vmm++) {
      for (int ch = 0; ch < CalibrationFile::MAX_CH; ch++) {
        ASSERT_FLOAT_EQ(cf.getCalibration(fec, vmm, ch).slope, 1.0);
        ASSERT_FLOAT_EQ(cf.getCalibration(fec, vmm, ch).offset, 0.0);
      }
    }
  }
}

TEST_F(CalibrationFileTest, GetCalibrationOutOfBounds) {
  CalibrationFile cf;
  auto &calib = cf.getCalibration(CalibrationFile::MAX_FEC, 0, 0);
  ASSERT_FLOAT_EQ(calib.slope, 0.0);
  ASSERT_FLOAT_EQ(calib.offset, 0.0);

  calib = cf.getCalibration(0, CalibrationFile::MAX_VMM, 0);
  ASSERT_FLOAT_EQ(calib.slope, 0.0);
  ASSERT_FLOAT_EQ(calib.offset, 0.0);

  calib = cf.getCalibration(0, 0, CalibrationFile::MAX_CH);
  ASSERT_FLOAT_EQ(calib.slope, 0.0);
  ASSERT_FLOAT_EQ(calib.offset, 0.0);
}

TEST_F(CalibrationFileTest, AddCalibrationOutOfBounds) {
  CalibrationFile cf;
  auto ret = cf.addCalibration(CalibrationFile::MAX_FEC, 0, 0, 1.0, 1.0);
  ASSERT_FLOAT_EQ(ret, false);
  ret = cf.addCalibration(0, CalibrationFile::MAX_VMM, 0, 1.0, 1.0);
  ASSERT_FLOAT_EQ(ret, false);
  ret = cf.addCalibration(0, 0, CalibrationFile::MAX_CH, 1.0, 1.0);
  ASSERT_FLOAT_EQ(ret, false);
}

TEST_F(CalibrationFileTest, AddCalibration) {
  CalibrationFile cf;
  int i = 0;
  for (int fec = 0; fec < CalibrationFile::MAX_FEC; fec++) {
    for (int vmm = 0; vmm < CalibrationFile::MAX_VMM; vmm++) {
      for (int ch = 0; ch < CalibrationFile::MAX_CH; ch++) {
        ASSERT_EQ(cf.addCalibration(fec, vmm, ch, 3.14159 + i, 2.71828 - i),
                  true);
        i++;
      }
    }
  }

  i = 0;
  for (int fec = 0; fec < CalibrationFile::MAX_FEC; fec++) {
    for (int vmm = 0; vmm < CalibrationFile::MAX_VMM; vmm++) {
      for (int ch = 0; ch < CalibrationFile::MAX_CH; ch++) {
        auto &calib = cf.getCalibration(fec, vmm, ch);
        ASSERT_FLOAT_EQ(calib.offset, 3.14159 + i);
        ASSERT_FLOAT_EQ(calib.slope, 2.71828 - i);
        i++;
      }
    }
  }
}

TEST_F(CalibrationFileTest, LoadCalibration) {
  CalibrationFile cf;
  cf.loadCalibration(DummyCal);
  auto cal = cf.getCalibration(1, 0, 0);
  ASSERT_FLOAT_EQ(cal.offset, 10.0);
  ASSERT_FLOAT_EQ(cal.slope, 1010.0);

  cal = cf.getCalibration(1, 0, 63);
  ASSERT_FLOAT_EQ(cal.offset, 10.7);
  ASSERT_FLOAT_EQ(cal.slope, 1010.7);

  cal = cf.getCalibration(1, 15, 0);
  ASSERT_FLOAT_EQ(cal.offset, 2.0);
  ASSERT_FLOAT_EQ(cal.slope, 3.0);

  cal = cf.getCalibration(1, 15, 63);
  ASSERT_FLOAT_EQ(cal.offset, 2.7);
  ASSERT_FLOAT_EQ(cal.slope, 3.7);
}

TEST_F(CalibrationFileTest, LoadCalibrationSizeMismatch) {
  CalibrationFile cf;
  cf.loadCalibration(ErrSizeMismatch);
  // These should be skipped and default to NoCal
  auto cal = cf.getCalibration(1, 0, 0);
  ASSERT_FLOAT_EQ(cal.offset, 0.0);
  ASSERT_FLOAT_EQ(cal.slope, 1.0);

  // These should be ok
  cal = cf.getCalibration(1, 15, 63);
  ASSERT_FLOAT_EQ(cal.offset, 2.7);
  ASSERT_FLOAT_EQ(cal.slope, 3.7);
}

TEST_F(CalibrationFileTest, LoadCalibrationFile) {
  std::string filename = "deleteme.json";
  DataSave tempfile(filename, (void *)DummyCal.c_str(), DummyCal.size());
  CalibrationFile cf(filename);

  auto cal = cf.getCalibration(1, 0, 0);
  ASSERT_FLOAT_EQ(cal.offset, 10.0);
  ASSERT_FLOAT_EQ(cal.slope, 1010.0);

  cal = cf.getCalibration(1, 0, 63);
  ASSERT_FLOAT_EQ(cal.offset, 10.7);
  ASSERT_FLOAT_EQ(cal.slope, 1010.7);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
