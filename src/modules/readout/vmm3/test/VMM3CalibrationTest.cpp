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

};

///\brief \todo using ASSERT_EQ for doubles and float might

TEST_F(VMM3CalibrationTest, Dummy) {
  ASSERT_EQ(1, 1);
}

TEST_F(VMM3CalibrationTest, Constructor) {
  VMM3Calibration cal;
  ASSERT_EQ(cal.ADCCorr(    0),    0);
  ASSERT_EQ(cal.ADCCorr(   42),   42);
  ASSERT_EQ(cal.ADCCorr( 1023), 1023);
  ASSERT_EQ(cal.ADCCorr( 1024), 1023);
  ASSERT_EQ(cal.ADCCorr(65535), 1023);

  ASSERT_EQ(cal.TDCCorr(  0), 1.5*22.72);
  ASSERT_EQ(cal.TDCCorr(255), 1.5*22.72 - 60);
}

TEST_F(VMM3CalibrationTest, ZeroCal) {
  VMM3Calibration cal(0.0, 0.0, 0.0, 0.0);
  ASSERT_EQ(cal.ADCCorr(   0), 0);
  ASSERT_EQ(cal.ADCCorr(  42), 0);
  ASSERT_EQ(cal.ADCCorr(1023), 0);

  ASSERT_EQ(cal.TDCCorr(  0), 0);
  ASSERT_EQ(cal.TDCCorr(255), 0);
}

TEST_F(VMM3CalibrationTest, ADCClampNegative) {
  VMM3Calibration cal(0.0, -1.0, 0.0, -1.0);
  ASSERT_EQ(cal.ADCCorr(    0), 0);
  ASSERT_EQ(cal.ADCCorr(   42), 0);
  ASSERT_EQ(cal.ADCCorr( 1023), 0);
  ASSERT_EQ(cal.ADCCorr( 1024), 0);
  ASSERT_EQ(cal.ADCCorr(65535), 0);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
