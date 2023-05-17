// Copyright (C) 2016-2022 European Spallation Source ERIC

#include <caen/geometry/Calibration.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

std::string NotJsonFile{"deleteme_caencalib_notjson.json"};
std::string NotJsonStr = R"(
  Failure is not an option.
)";

/// \brief straws should go 0, 1, 2, 3, ...
auto BadStrawOrder = R"(
  {
    "CaenCalibration" : {
      "ntubes" : 1,
      "nstraws" : 3,
      "resolution" : 512,

      "polynomials" : [
        [0,0.0,0.0,0.0,0.0],
        [2,0.0,0.0,0.0,0.0],
        [1,0.0,0.0,0.0,0.0]
      ]
    }
  }
)"_json;

/// \brief three calibration entries and four straws promised
auto StrawMismatch = R"(
  {
    "CaenCalibration" : {
      "ntubes" : 1,
      "nstraws" : 4,
      "resolution" : 512,

      "polynomials" : [
        [0,0.0,0.0,0.0,0.0],
        [1,0.0,0.0,0.0,0.0],
        [2,0.0,0.0,0.0,0.0]
      ]
    }
  }
)";

/// \brief one entry has too few coefficients
auto InvalidCoeff = R"(
  {
    "CaenCalibration" : {
      "ntubes" : 1,
      "nstraws" : 3,
      "resolution" : 512,

      "polynomials" : [
        [0, 0.0, 0.0, 0.0, 0.0],
        [1, 0.0, 0.0, 0.0],
        [2, 0.0, 0.0, 0.0, 0.0]
      ]
    }
  }
)"_json;


auto StrawMappingNull = R"(
  {
    "LokiCalibration" : {
      "ntubes" : 1,
      "nstraws" : 3,
      "resolution" : 256,

      "polynomials" : [
        [0, 0.0, 1.0, 0.0, 0.0],
        [1, 0.0, 1.0, 0.0, 0.0],
        [2, 0.0, 1.0, 0.0, 0.0]
      ]
    }
  }
)"_json;

auto StrawMappingConst = R"(
  {
    "LokiCalibration" : {
      "ntubes" : 1,
      "nstraws" : 3,
      "resolution" : 256,

      "polynomials" : [
        [0, 0.0, 0.0, 0.0, 0.0],
        [1, 1.0, 0.0, 0.0, 0.0],
        [2, 2.0, 0.0, 0.0, 0.0]
      ]
    }
  }
)"_json;

/// Adding a BIFROST calibration file
auto BifrostGood = R"(
  {
    "BifrostCalibration":
    {
      "Intervals" :
      [
        [ 0, 0.001, 0.333, 0.333, 0.667, 0.667, 1.000],
        [ 1, 0.002, 0.333, 0.333, 0.667, 0.667, 1.000],
        [ 2, 0.003, 0.333, 0.333, 0.667, 0.667, 1.000]
      ]
    }
  }
)"_json;

auto BifrostBadNames = R"(
  {
    "BadName":
    {
      "BadIntervals" :
      [
        [ 0, 0.001, 0.333, 0.333, 0.667, 0.667, 1.000],
        [ 1, 0.002, 0.333, 0.333, 0.667, 0.667, 1.000],
        [ 2, 0.003, 0.333, 0.333, 0.667, 0.667, 1.000]
      ]
    }
  }
)"_json;

auto BifrostBadTripletId = R"(
  {
    "BifrostCalibration":
    {
      "Intervals" :
      [
        [ 0,  0.001, 0.333, 0.333, 0.667, 0.667, 1.000],
        [ 45, 0.002, 0.333, 0.333, 0.667, 0.667, 1.000],
        [ 46, 0.003, 0.333, 0.333, 0.667, 0.667, 1.000]
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
  Calibration calib;
  ASSERT_EQ(calib.StrawCalibration.size(), 0);
  ASSERT_EQ(calib.getMaxPixel(), 0);
}

TEST_F(CalibrationTest, NullCalibrationWrongSizeStraw) {
  Calibration calib;
  uint32_t BadStraws{6};
  uint16_t Resolution{256};
  ASSERT_ANY_THROW(calib.nullCalibration(BadStraws, Resolution));
}

TEST_F(CalibrationTest, NullCalibrationWrongSizeResolution) {
  Calibration calib;
  uint32_t Straws{7};
  uint16_t BadResolution{255};
  ASSERT_ANY_THROW(calib.nullCalibration(Straws, BadResolution));

  BadResolution = 1025;
  ASSERT_ANY_THROW(calib.nullCalibration(Straws, BadResolution));
}

TEST_F(CalibrationTest, NullCalibrationGood) {
  Calibration calib;
  uint32_t Straws{6160};
  uint16_t Resolution{256};
  calib.nullCalibration(Straws, Resolution);
  for (uint32_t Straw = 0; Straw < Straws; Straw++) {
    for (uint32_t Pos = 0; Pos < Resolution; Pos++) {
      ASSERT_EQ(calib.strawCorrection(Straw, Pos), Pos);
    }
  }
}

// Test clamping to 0 and max by manipulating polynomial coefficients
TEST_F(CalibrationTest, ClampLowAndHigh) {
  Calibration calib;
  calib.root = StrawMappingNull;
  calib.loadLokiParameters();
  int64_t ClampLow = 0;
  int64_t ClampHigh = 0;
  calib.Stats.ClampLow = &ClampLow;
  calib.Stats.ClampHigh = &ClampHigh;

  calib.StrawCalibration[0][0] = 100.0; // a = 100
  uint32_t res = calib.strawCorrection(0, 5.0);
  ASSERT_EQ(*calib.Stats.ClampLow, 1);
  ASSERT_EQ(res, 0);

  calib.StrawCalibration[0][0] = -2000;
  res = calib.strawCorrection(0, 5.0);
  ASSERT_EQ(*calib.Stats.ClampHigh, 1);
  ASSERT_EQ(res, 255);
}

TEST_F(CalibrationTest, LoadCalib) {
  Calibration calib;
  calib.root = StrawMappingNull;
  calib.loadLokiParameters();
  ASSERT_EQ(calib.StrawCalibration.size(), 3);
  ASSERT_EQ(calib.getMaxPixel(), 3 * 256);
}

TEST_F(CalibrationTest, LoadCalibConst) {
  uint32_t Straws{3};
  uint16_t Resolution{256};

  Calibration calib;
  calib.root = StrawMappingConst;
  calib.loadLokiParameters();
  ASSERT_EQ(calib.StrawCalibration.size(), Straws);
  ASSERT_EQ(calib.getMaxPixel(), Straws * Resolution);

  for (uint32_t Straw = 0; Straw < Straws; Straw++) {
    for (uint32_t Pos = 2; Pos < Resolution; Pos++) {

      ASSERT_EQ(calib.strawCorrection(Straw, Pos), Pos - Straw);
    }
  }
}

TEST_F(CalibrationTest, NOTJson) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  ASSERT_ANY_THROW(Calibration calib = Calibration(NotJsonFile));
  deleteFile(NotJsonFile);
}

TEST_F(CalibrationTest, BadStrawOrder) {
  Calibration calib;
  calib.root = BadStrawOrder;
  ASSERT_ANY_THROW(calib.loadLokiParameters());
}

TEST_F(CalibrationTest, StrawMismatch) {
  Calibration calib;
  calib.root = StrawMismatch;
  ASSERT_ANY_THROW(calib.loadLokiParameters());
}

TEST_F(CalibrationTest, InvalidCoeff) {
  Calibration calib;
  calib.root = InvalidCoeff;
  ASSERT_ANY_THROW(calib.loadLokiParameters());
}

TEST_F(CalibrationTest, BifrostGood) {
  Calibration calib;
  calib.root = BifrostGood;
  calib.loadBifrostParameters();
  ASSERT_NEAR(calib.BifrostCalibration.TripletCalib[0][0], 0.001, 0.0001);
}

TEST_F(CalibrationTest, BifrostBadNames) {
  Calibration calib;
  calib.root = BifrostBadNames;
  ASSERT_ANY_THROW(calib.loadBifrostParameters());
}

TEST_F(CalibrationTest, BifrostBadTripletId) {
  Calibration calib;
  calib.root = BifrostBadTripletId;
  ASSERT_ANY_THROW(calib.loadBifrostParameters());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto retval = RUN_ALL_TESTS();
  return retval;
}
