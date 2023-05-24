// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

#include <caen/geometry/CDCalibration.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

///
auto LokiExample = R"(
  {
    "Calibration":
    {
      "instrument" : "loki",
      "groups" : 2,
      "groupsize" : 7,
      "amplitudes" : 4,
      "pixellation" : 512,
      "default intervals" : [0.000, 0.142, 0.143, 0.285, 0.286, 0.428, 0.429, 0.571, 0.572, 0.714, 0.715, 0.857, 0.858, 1.0],

      "Parameters" : [
        {
          "groupindex" : 0,
          "intervals"  : [0.0, 0.142, 0.143, 0.285, 0.286, 0.428, 0.429, 0.571, 0.572, 0.714, 0.715, 0.857, 0.858, 1.0],
          "polynomials" : [[0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0]]
        },
        {
          "groupindex" : 1,
          "intervals"  : [0.0, 0.142, 0.143, 0.285, 0.286, 0.428, 0.429, 0.571, 0.572, 0.714, 0.715, 0.857, 0.858, 1.0],
          "polynomials" : [[0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0]]
        }
      ]
    }
  }
)"_json;


auto ErrorDefInterval = R"(
  {
    "Calibration":
    {
      "instrument" : "loki",
      "groups" : 1, "groupsize" : 1, "amplitudes" : 2, "pixellation" : 512,
      "default intervals" : [0.000, 0.142, 0.500],

      "Parameters" : [
        { "groupindex" : 0, "intervals"  : [0.0, 0.142], "polynomials" : [[0.0, 0.0, 0.0, 0.0]] },
        { "groupindex" : 1, "intervals"  : [0.0, 0.142], "polynomials" : [[0.0, 0.0, 0.0, 0.0]] }
      ]
    }
  }
)"_json;

using namespace Caen;

class CalibrationTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(CalibrationTest, Constructor) {
  CDCalibration calib("loki");
  calib.root = LokiExample;
  calib.parseCalibration();
}

TEST_F(CalibrationTest, BadName) {
  CDCalibration calib("wotan");
  calib.root = LokiExample;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, BadNumberOfGroups) {
  CDCalibration calib("loki");
  calib.root = LokiExample;
  // fake invalid number of groups from otherwise valid file
  calib.root["Calibration"]["groups"] = 5;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, BadNumberOfGroupIntervals) {
  CDCalibration calib("loki");
  calib.root = LokiExample;
  // fake invalid number of intervals for group 0 from otherwise valid file
  calib.root["Calibration"]["Parameters"][0]["intervals"].push_back(0.1);
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, ErrPosNotInUnitInterval) {
  CDCalibration calib("loki");
  calib.root = LokiExample;
  // fake invalid interval value for group 0 from otherwise valid file
  calib.root["Calibration"]["Parameters"][0]["intervals"][0] = -0.0001;
  ASSERT_ANY_THROW(calib.parseCalibration());

  calib.root = LokiExample;
  calib.parseCalibration(); // check that we've copied in a new valid calib

  calib.root["Calibration"]["Parameters"][0]["intervals"][1] = 1.001;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, ErrPosUnordered) {
  CDCalibration calib("loki");
  calib.root = LokiExample;
  // fake misordered interval values for group 0 from otherwise valid file
  calib.root["Calibration"]["Parameters"][0]["intervals"][0] = 0.5;
  calib.root["Calibration"]["Parameters"][0]["intervals"][0] = 0.4;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, ErrPolynomialVectorSize) {
  CDCalibration calib("loki");
  calib.root = LokiExample;
  // fake bad size of polynomial vector for group 0 from otherwise valid file
  calib.root["Calibration"]["Parameters"][0]["polynomials"].push_back(0.1);
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, ErrCoefficientVectorSize) {
  CDCalibration calib("loki");
  calib.root = LokiExample;
  // fake bad size of coefficient vector for group 0/groupindex 0 from otherwise valid file
  calib.root["Calibration"]["Parameters"][0]["polynomials"][0].push_back(0.1);
  ASSERT_ANY_THROW(calib.parseCalibration());
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto retval = RUN_ALL_TESTS();
  return retval;
}
