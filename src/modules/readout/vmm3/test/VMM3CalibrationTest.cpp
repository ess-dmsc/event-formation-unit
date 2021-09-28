// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for VMM3Calibration
///
//===----------------------------------------------------------------------===//

#include <readout/vmm3/VMM3Calibration.h>
#include <test/TestBase.h>


class VMM3CalibrationTest : public TestBase {
protected:
  VMM3Calibration cal;
};

///\brief \todo using ASSERT_EQ for doubles and float might
/// not be accurate

TEST_F(VMM3CalibrationTest, Constructor) {
  for (int ch = 0; ch < VMM3Calibration::CHANNELS; ch++) {
    ASSERT_EQ(cal.ADCCorr(ch,     0),    0);
    ASSERT_EQ(cal.ADCCorr(ch,    42),   42);
    ASSERT_EQ(cal.ADCCorr(ch,  1023), 1023);
    ASSERT_EQ(cal.ADCCorr(ch,  1024), 1023);
    ASSERT_EQ(cal.ADCCorr(ch, 65535), 1023);

    ASSERT_EQ(cal.TDCCorr(ch,   0), 1.5 * 22.72     );
    ASSERT_EQ(cal.TDCCorr(ch, 255), 1.5 * 22.72 - 60);
  }
}

TEST_F(VMM3CalibrationTest, ZeroCal) {
  for (int ch = 0; ch < VMM3Calibration::CHANNELS; ch++) {
    cal.setCalibration(ch, 0.0, 0.0, 0.0, 0.0);
    ASSERT_EQ(cal.ADCCorr(ch,    0), 0);
    ASSERT_EQ(cal.ADCCorr(ch,   42), 0);
    ASSERT_EQ(cal.ADCCorr(ch, 1023), 0);

    ASSERT_EQ(cal.TDCCorr(ch,   0), 0);
    ASSERT_EQ(cal.TDCCorr(ch, 255), 0);
  }
}

TEST_F(VMM3CalibrationTest, ADCClampNegative) {
  for (int ch = 0; ch < VMM3Calibration::CHANNELS; ch++) {
    cal.setCalibration(ch, 0.0, -1.0, 0.0, -1.0);
    ASSERT_EQ(cal.ADCCorr(ch,     0), 0);
    ASSERT_EQ(cal.ADCCorr(ch,    42), 0);
    ASSERT_EQ(cal.ADCCorr(ch,  1023), 0);
    ASSERT_EQ(cal.ADCCorr(ch,  1024), 0);
    ASSERT_EQ(cal.ADCCorr(ch, 65535), 0);
 }
}

TEST_F(VMM3CalibrationTest, ADCClampPositive) {
  for (int ch = 0; ch < VMM3Calibration::CHANNELS; ch++) {
    cal.setCalibration(ch, 0.0, -1.0, -1.0, 2000.0);
    ASSERT_EQ(cal.ADCCorr(ch,     0), 1023);
    ASSERT_EQ(cal.ADCCorr(ch,    42), 1023);
    ASSERT_EQ(cal.ADCCorr(ch,  1023), 1023);
    ASSERT_EQ(cal.ADCCorr(ch,  1024), 1023);
    ASSERT_EQ(cal.ADCCorr(ch, 65535), 1023);
  }
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
