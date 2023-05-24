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
      "groups" : 2, "groupsize" : 1, "amplitudes" : 2, "pixellation" : 512,
      "default intervals" : [0.000, 0.142, 0.500],

      "Parameters" : [
        { "groupindex" : 0, "intervals"  : [0.0, 0.142], "polynomials" : [[0.0, 0.0, 0.0, 0.0]] },
        { "groupindex" : 1, "intervals"  : [0.0, 0.142], "polynomials" : [[0.0, 0.0, 0.0, 0.0]] }
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
      "default intervals" : [0.000, 0.142],

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
      "default intervals" : [0.000, 0.100, 0.200, 0.300],

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


TEST_F(CalibrationTest, Constructor) {
  calib.parseCalibration();
}

TEST_F(CalibrationTest, NotJsonFile) {
  ASSERT_ANY_THROW(calib = CDCalibration("loki", InvalidJsonName));
}

TEST_F(CalibrationTest, BadName) {
  calib = CDCalibration("wotan");
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


TEST_F(CalibrationTest, BadNumberOfDefaultIntervals) {
  // fake invalid number of groups from otherwise valid file
  calib.root["Calibration"]["default intervals"].push_back(42.0);
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, BadNumberOfGroupIntervals) {
  // fake invalid number of intervals for group 0 from otherwise valid file
  calib.root["Calibration"]["Parameters"][0]["intervals"].push_back(0.1);
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, ErrPosNotInUnitInterval) {
  // set valid outer ranges for group 0
  calib.root["Calibration"]["Parameters"][0]["intervals"][0] = 0.000;
  calib.root["Calibration"]["Parameters"][0]["intervals"][1] = 1.000;

  // fake invalid interval value for group 1 from otherwise valid file
  calib.root["Calibration"]["Parameters"][1]["intervals"][0] = -0.0001;
  ASSERT_ANY_THROW(calib.parseCalibration());

  calib.root = LokiExample;
  calib.parseCalibration(); // check that we've copied in a new valid calib

  calib.root["Calibration"]["Parameters"][1]["intervals"][1] = 1.001;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, ErrPosUnordered) {
  // fake misordered interval values for group 0 from otherwise valid file
  calib.root["Calibration"]["Parameters"][0]["intervals"][0] = 0.5;
  calib.root["Calibration"]["Parameters"][0]["intervals"][1] = 0.4;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, ErrPolynomialVectorSize) {
  calib.root = ErrorGroupSizeMismatch;
  ASSERT_ANY_THROW(calib.parseCalibration());
}

TEST_F(CalibrationTest, ErrCoefficientVectorSize) {
  // fake bad size of coefficient vector for group 0/groupindex 0 from otherwise valid file
  calib.root["Calibration"]["Parameters"][0]["polynomials"][0].push_back(0.1);
  ASSERT_ANY_THROW(calib.parseCalibration());
}


int main(int argc, char **argv) {
  saveBuffer(InvalidJsonName, (void *)InvalidJson.c_str(), InvalidJson.size());
  testing::InitGoogleTest(&argc, argv);
  auto retval = RUN_ALL_TESTS();
  return retval;
}
