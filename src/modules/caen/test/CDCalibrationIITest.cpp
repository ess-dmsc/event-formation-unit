// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

#include <caen/geometry/CDCalibration.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>


// clang-format off
///
auto SimplePolynomials = R"(
  {
    "Calibration":
    {
      "instrument" : "dummy",
      "groups" : 4,
      "groupsize" : 1,

      "Parameters" : [
        {
          "groupindex" : 0,
          "intervals"  : [[0.0, 1.0]],
          "polynomials" : [[0.00, 0.00, 0.00, 0.00]]
        },
        {
          "groupindex" : 1,
          "intervals"  : [[0.0, 1.0]],
          "polynomials" : [[1.0, 0.0, 0.0, 0.0]]
        },
        {
          "groupindex" : 2,
          "intervals"  : [[0.0, 1.0]],
          "polynomials" : [[0.0, 1.0, 0.0, 0.0]]
        },
        {
          "groupindex" : 3,
          "intervals"  : [[0.0, 1.0]],
          "polynomials" : [[-1.0, 0.0, 0.0, 0.0]]
        }
      ]
    }
  }
)"_json;


// clang-format on

using namespace Caen;

class CalibrationIITest : public TestBase {
protected:
  CDCalibration calib{"dummy"};

  void SetUp() override {
      calib.root = SimplePolynomials;
  }
  void TearDown() override {}
};


// Test basic polynomial calibrations
TEST_F(CalibrationIITest, SelectedValues) {
  calib.parseCalibration();
  // Null calibration
  ASSERT_NEAR(calib.posCorrection(0, 0, 0.0),   0, 0.001);
  ASSERT_NEAR(calib.posCorrection(0, 0, 0.5), 0.5, 0.001);
  ASSERT_NEAR(calib.posCorrection(0, 0, 1.0), 1.0, 0.001);

  // Constant subtraction of 1.0
  ASSERT_NEAR(calib.posCorrection(1, 0, 0.0), 0, 0.001);
  ASSERT_EQ(calib.Stats.ClampLow, 1);
  ASSERT_NEAR(calib.posCorrection(1, 0, 1.0), 0, 0.001);

  // Identity subtraction - all values are zero
  for (int i = 0; i <= 100; i++) {
    ASSERT_NEAR(calib.posCorrection(2, 0, i/100.0), 0, 0.001);
  }

  // Constant addition of 1.0
  ASSERT_NEAR(calib.posCorrection(3, 0, 0.0), 1.0, 0.001);
  ASSERT_EQ(calib.Stats.ClampHigh, 0);
  ASSERT_NEAR(calib.posCorrection(3, 0, 1.0), 1.0, 0.001);
  ASSERT_EQ(calib.Stats.ClampHigh, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto retval = RUN_ALL_TESTS();
  return retval;
}
