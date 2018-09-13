/** Copyright (C) 2018 European Spallation Source ERIC */

#include <gdgem/vmm3/CalibrationFile.h>
#include <test/TestBase.h>
#include <vector>

// std::vector<CalibrationFile::calibration_t> testCorrection = {
//   {0.0, 1.0}, {0.0, 2.0}, {0.0, 3.0}, {0.0, 4.0}, {0.0, 5.0}, {0.0, 6.0}, {0.0, 7.0}, {0.0, 8.0},
//   {1.0, 1.0}, {1.0, 2.0}, {1.0, 3.0}, {1.0, 4.0}, {1.0, 5.0}, {1.0, 6.0}, {1.0, 7.0}, {1.0, 8.0},
//   {2.0, 1.0}, {2.0, 2.0}, {2.0, 3.0}, {2.0, 4.0}, {2.0, 5.0}, {2.0, 6.0}, {2.0, 7.0}, {2.0, 8.0},
//   {3.0, 1.0}, {3.0, 2.0}, {3.0, 3.0}, {3.0, 4.0}, {3.0, 5.0}, {3.0, 6.0}, {3.0, 7.0}, {3.0, 8.0},
//   {4.0, 1.0}, {4.0, 2.0}, {4.0, 3.0}, {4.0, 4.0}, {4.0, 5.0}, {4.0, 6.0}, {4.0, 7.0}, {4.0, 8.0},
//   {5.0, 1.0}, {5.0, 2.0}, {5.0, 3.0}, {5.0, 4.0}, {5.0, 5.0}, {5.0, 6.0}, {5.0, 7.0}, {5.0, 8.0},
//   {7.0, 1.0}, {6.0, 2.0}, {6.0, 3.0}, {6.0, 4.0}, {6.0, 5.0}, {6.0, 6.0}, {6.0, 7.0}, {6.0, 8.0},
//   {7.0, 1.0}, {7.0, 2.0}, {7.0, 3.0}, {7.0, 4.0}, {7.0, 5.0}, {7.0, 6.0}, {7.0, 7.0}, {7.0, 8.0}
// };


class CalibrationFileTest : public TestBase {

protected:
  virtual void SetUp() {  }
  virtual void TearDown() {  }
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
  auto & calib = cf.getCalibration(CalibrationFile::MAX_FEC, 0, 0);
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
  ASSERT_FLOAT_EQ(ret, -1);
  ret = cf.addCalibration(0, CalibrationFile::MAX_VMM, 0, 1.0, 1.0);
  ASSERT_FLOAT_EQ(ret, -1);
  ret = cf.addCalibration(0, 0, CalibrationFile::MAX_CH, 1.0, 1.0);
  ASSERT_FLOAT_EQ(ret, -1);
}

TEST_F(CalibrationFileTest, AddCalibration) {
  CalibrationFile cf;
  int i = 0;
  for (int fec = 0; fec < CalibrationFile::MAX_FEC; fec++) {
    for (int vmm = 0; vmm < CalibrationFile::MAX_VMM; vmm++) {
      for (int ch = 0; ch < CalibrationFile::MAX_CH; ch++) {
        ASSERT_EQ(cf.addCalibration(fec, vmm, ch, 3.14159 + i, 2.71828 - i), 0);
        i++;
      }
    }
  }

  i = 0;
  for (int fec = 0; fec < CalibrationFile::MAX_FEC; fec++) {
    for (int vmm = 0; vmm < CalibrationFile::MAX_VMM; vmm++) {
      for (int ch = 0; ch < CalibrationFile::MAX_CH; ch++) {
        auto & calib = cf.getCalibration(fec, vmm, ch);
        ASSERT_FLOAT_EQ(calib.offset, 3.14159 + i);
        ASSERT_FLOAT_EQ(calib.slope, 2.71828 - i);
        i++;
      }
    }
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
