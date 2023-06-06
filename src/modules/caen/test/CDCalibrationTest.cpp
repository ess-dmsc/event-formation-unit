// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

#include <caen/geometry/CDCalibration.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>


std::string InvalidJsonName{"deleteme_cdcalib.json"};
std::string InvalidJson = R"(
  Not good enough for Gudonoff!
)";

// clang-format off
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

      "Parameters" : [
        {
          "groupindex" : 0,
          "intervals"  : [[0.0, 0.142], [0.143, 0.285], [0.286, 0.428], [0.429, 0.571], [0.572, 0.714], [0.715, 0.857], [0.858, 1.0]],
          "polynomials" : [[0.00, 0.01, 0.02, 0.03], [0.04, 0.05, 0.06, 0.07], [0.08, 0.09, 0.10, 0.11], [0.12, 0.13, 0.14, 0.15], [0.16, 0.17, 0.18, 0.19], [0.20, 0.21, 0.22, 0.23], [0.24, 0.25, 0.26, 0.27]]
        },
        {
          "groupindex" : 1,
          "intervals"  : [[0.0, 0.142], [0.143, 0.285], [0.286, 0.428], [0.429, 0.571], [0.572, 0.714], [0.715, 0.857], [0.858, 1.0]],
          "polynomials" : [[0.28, 0.29, 0.30, 0.31], [0.32, 0.33, 0.34, 0.35], [0.36, 0.37, 0.38, 0.39], [0.40, 0.41, 0.42, 0.43], [0.44, 0.45, 0.46, 0.47], [0.48, 0.49, 0.50, 0.51], [0.52, 0.53, 0.54, 0.55]]
        }
      ]
    }
  }
)"_json;


auto ErrorMissingCalibration = R"(
  {
    "Ca li bra tion":
    {
      "instrument" : "loki",
      "groups" : 1, "groupsize" : 1, "amplitudes" : 2, "pixellation" : 512,

      "Parameters" : [
        { "groupindex" : 0, "intervals"  : [0.0, 0.142], "polynomials" : [[0.0, 0.0, 0.0, 0.0]] },
        { "groupindex" : 1, "intervals"  : [0.0, 0.142], "polynomials" : [[0.0, 0.0, 0.0, 0.0]] }
      ]
    }
  }
)"_json;


auto ErrorGroupSizeMismatch = R"(
  {
    "Calibration":
    {
      "instrument" : "loki",
      "groups" : 2, "groupsize" : 2, "amplitudes" : 2, "pixellation" : 512,

      "Parameters" : [
        { "groupindex" : 0, "intervals"  : [0.000, 0.100, 0.200, 0.300], "polynomials" : [[0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0]] },
        { "groupindex" : 1, "intervals"  : [0.000, 0.100, 0.200, 0.300], "polynomials" : [[0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0, 0.0]] }
      ]
    }
  }
)"_json;
// clang-format on

using namespace Caen;

class CalibrationTest : public TestBase {
protected:
  CDCalibration calib{"loki"};

  void SetUp() override {
      calib.root = LokiExample;
  }
  void TearDown() override {}
};

#if 0

TEST_F(CalibrationTest, NotJsonFile) {
  ASSERT_ANY_THROW(calib = CDCalibration("loki", InvalidJsonName));
}

TEST_F(CalibrationTest, BadName) {
  calib = CDCalibration("wotan");
  calib.root = LokiExample;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, MissingCalibrationEntry) {
  calib.root = ErrorMissingCalibration;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, BadNumberOfGroups) {
  // fake invalid number of groups from otherwise valid file
  calib.root["Calibration"]["groups"] = 5;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, BadGroupIndex) {
  // fake unordered groups from otherwise valid file
  calib.root["Calibration"]["Parameters"][1]["groupindex"] = 2;
  ASSERT_ANY_THROW(calib.parseCalibration());
}


TEST_F(CalibrationTest, BadNumberOfGroupIntervals) {
  // fake invalid number of intervals for group 0 from otherwise valid file
  std::pair<double, double> NewPair{0.1, 0.2};
  calib.root["Calibration"]["Parameters"][0]["intervals"].push_back(NewPair);
  ASSERT_ANY_THROW(calib.parseCalibration());
}
#endif


TEST_F(CalibrationTest, ErrPosNotInUnitInterval) {
  // set valid outer ranges for group 0
  calib.root["Calibration"]["Parameters"][0]["intervals"][0][0] = 0.000;
  calib.root["Calibration"]["Parameters"][0]["intervals"][0][1] = 1.000;

  // fake invalid interval value for group 1 from otherwise valid file
  calib.root["Calibration"]["Parameters"][1]["intervals"][0][0] = -0.0001;
  ASSERT_ANY_THROW(calib.parseCalibration());

  calib.root = LokiExample;
  calib.parseCalibration(); // check that we've copied in a new valid calib

  calib.root["Calibration"]["Parameters"][1]["intervals"][0][1] = 1.001;
  ASSERT_ANY_THROW(calib.parseCalibration());
}


// TEST_F(CalibrationTest, ErrPosUnordered) {
//   // fake misordered interval values for group 0 from otherwise valid file
//   calib.root["Calibration"]["Parameters"][0]["intervals"][0] = 0.5;
//   calib.root["Calibration"]["Parameters"][0]["intervals"][1] = 0.4;
//   ASSERT_ANY_THROW(calib.parseCalibration());
// }
#if 0
TEST_F(CalibrationTest, ErrPolynomialVectorSize) {
  calib.root = ErrorGroupSizeMismatch;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, ErrCoefficientVectorSize) {
  // fake bad size of coefficient vector for group 0/groupindex 0 from otherwise valid file
  calib.root["Calibration"]["Parameters"][0]["polynomials"][0].push_back(0.1);
  ASSERT_ANY_THROW(calib.parseCalibration());
}



// Test that after loading we have the correct polynomial values
TEST_F(CalibrationTest, LokiTest) {
  calib.parseCalibration();
  int Entries{0};
  for (int i = 0; i < calib.Parms.Groups; i++) {
    for (int j = 0; j < calib.Parms.GroupSize; j++) {
      for (int k = 0; k < 4; k++) {
        EXPECT_NEAR(calib.Calibration[i][j][k], 1.0*Entries/100, 0.0001);
        Entries++;
      }

    }
  }
}
#endif

int main(int argc, char **argv) {
  saveBuffer(InvalidJsonName, (void *)InvalidJson.c_str(), InvalidJson.size());
  testing::InitGoogleTest(&argc, argv);
  auto retval = RUN_ALL_TESTS();
  return retval;
}
