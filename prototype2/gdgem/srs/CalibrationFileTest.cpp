/** Copyright (C) 2018 European Spallation Source ERIC */

#include <gdgem/srs/CalibrationFile.h>
#include <gdgem/srs/CalibrationFileTestData.h>
#include <prototype2/common/DataSave.h>
#include <test/TestBase.h>
#include <vector>

using namespace Gem;

class CalibrationFileTest : public TestBase {
protected:
  // virtual void SetUp() {  }
  // virtual void TearDown() {  }
};

/** Test cases below */
TEST_F(CalibrationFileTest, Constructor) {
  CalibrationFile cf;
  for (size_t fec = 0; fec < CalibrationFile::MAX_FEC; fec++) {
    for (size_t vmm = 0; vmm < CalibrationFile::MAX_VMM; vmm++) {
      for (size_t ch = 0; ch < CalibrationFile::MAX_CH; ch++) {
        EXPECT_FLOAT_EQ(cf.getCalibration(fec, vmm, ch).slope, 1.0);
        EXPECT_FLOAT_EQ(cf.getCalibration(fec, vmm, ch).offset, 0.0);
      }
    }
  }
}

TEST_F(CalibrationFileTest, AddCalibration) {
  CalibrationFile cf;
  int i = 0;
  for (size_t fec = 0; fec < CalibrationFile::MAX_FEC; fec++) {
    for (size_t vmm = 0; vmm < CalibrationFile::MAX_VMM; vmm++) {
      for (size_t ch = 0; ch < CalibrationFile::MAX_CH; ch++) {
        cf.addCalibration(fec, vmm, ch, 3.14159 + i, 2.71828 - i);
        i++;
      }
    }
  }

  i = 0;
  for (size_t fec = 0; fec < CalibrationFile::MAX_FEC; fec++) {
    for (size_t vmm = 0; vmm < CalibrationFile::MAX_VMM; vmm++) {
      for (size_t ch = 0; ch < CalibrationFile::MAX_CH; ch++) {
        auto calib = cf.getCalibration(fec, vmm, ch);
        EXPECT_FLOAT_EQ(calib.offset, 3.14159 + i);
        EXPECT_FLOAT_EQ(calib.slope, 2.71828 - i);
        i++;
      }
    }
  }
}

TEST_F(CalibrationFileTest, LoadCalibrationInvalidJsonFile) {
  CalibrationFile cf;
  EXPECT_THROW(cf.loadCalibration(InvalidJson), std::runtime_error);
}

TEST_F(CalibrationFileTest, LoadCalibrationInvalidOffsetField) {
  CalibrationFile cf;
  EXPECT_THROW(cf.loadCalibration(InvalidJson), std::runtime_error);
}

TEST_F(CalibrationFileTest, LoadCalibration) {
  CalibrationFile cf;
  cf.loadCalibration(DummyCal);
  auto cal = cf.getCalibration(1, 0, 0);
  EXPECT_FLOAT_EQ(cal.offset, 10.0);
  EXPECT_FLOAT_EQ(cal.slope, 1010.0);

  cal = cf.getCalibration(1, 0, 63);
  EXPECT_FLOAT_EQ(cal.offset, 10.7);
  EXPECT_FLOAT_EQ(cal.slope, 1010.7);

  cal = cf.getCalibration(1, 15, 0);
  EXPECT_FLOAT_EQ(cal.offset, 2.0);
  EXPECT_FLOAT_EQ(cal.slope, 3.0);

  cal = cf.getCalibration(1, 15, 63);
  EXPECT_FLOAT_EQ(cal.offset, 2.7);
  EXPECT_FLOAT_EQ(cal.slope, 3.7);
}

TEST_F(CalibrationFileTest, LoadCalibrationSizeMismatch) {
  CalibrationFile cf;
  EXPECT_THROW(cf.loadCalibration(ErrSizeMismatch), std::runtime_error);
}

TEST_F(CalibrationFileTest, LoadCalibrationFile) {
  std::string filename = "deleteme.json";
  DataSave tempfile(filename, (void *)DummyCal.c_str(), DummyCal.size());
  CalibrationFile cf(filename);

  auto cal = cf.getCalibration(1, 0, 0);
  EXPECT_FLOAT_EQ(cal.offset, 10.0);
  EXPECT_FLOAT_EQ(cal.slope, 1010.0);

  cal = cf.getCalibration(1, 0, 63);
  EXPECT_FLOAT_EQ(cal.offset, 10.7);
  EXPECT_FLOAT_EQ(cal.slope, 1010.7);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
