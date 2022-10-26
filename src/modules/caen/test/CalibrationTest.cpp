/** Copyright (C) 2016-2022 European Spallation Source ERIC */

#include <caen/geometry/Calibration.h>
#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>

std::string NotJsonFile{"deleteme_caencalib_notjson.json"};
std::string NotJsonStr = R"(
  Failure is not an option.
)";

/// \brief straws should go 0, 1, 2, 3, ...
std::string BadStrawOrderFile{"deleteme_caencalib_badstraworder.json"};
std::string BadStrawOrderStr = R"(
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
)";

/// \brief three calibration entries and four straws promised
std::string StrawMismatchFile{"deleteme_caencalib_strawmismatch.json"};
std::string StrawMismatchStr = R"(
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
std::string InvalidCoeffFile{"deleteme_caencalib_invalidcoeff.json"};
std::string InvalidCoeffStr = R"(
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
)";

std::string StrawMappingNullFile{"deleteme_caencalib_strawmapping_null.json"};
std::string StrawMappingNullStr = R"(
  {
    "CaenCalibration" : {
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
)";

std::string StrawMappingConstFile{
    "deleteme_caencalib_strawmapping_strawid.json"};
std::string StrawMappingConstStr = R"(
  {
    "CaenCalibration" : {
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
)";

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
  saveBuffer(StrawMappingNullFile, (void *)StrawMappingNullStr.c_str(),
             StrawMappingNullStr.size());
  Calibration calib = Calibration(StrawMappingNullFile);
  calib.Stats.ClampLow = new int64_t;
  calib.Stats.ClampHigh = new int64_t;

  calib.StrawCalibration[0][0] = 100.0; // a = 100
  uint32_t res = calib.strawCorrection(0, 5.0);
  ASSERT_EQ(*calib.Stats.ClampLow, 1);
  ASSERT_EQ(res, 0);

  calib.StrawCalibration[0][0] = -2000;
  res = calib.strawCorrection(0, 5.0);
  ASSERT_EQ(*calib.Stats.ClampHigh, 1);
  ASSERT_EQ(res, 255);

  deleteFile(StrawMappingNullFile);
}

TEST_F(CalibrationTest, LoadCalib) {
  saveBuffer(StrawMappingNullFile, (void *)StrawMappingNullStr.c_str(),
             StrawMappingNullStr.size());
  Calibration calib = Calibration(StrawMappingNullFile);
  ASSERT_EQ(calib.StrawCalibration.size(), 3);
  ASSERT_EQ(calib.getMaxPixel(), 3 * 256);
  deleteFile(StrawMappingNullFile);
}

TEST_F(CalibrationTest, LoadCalibConst) {
  saveBuffer(StrawMappingConstFile, (void *)StrawMappingConstStr.c_str(),
             StrawMappingConstStr.size());

  uint32_t Straws{3};
  uint16_t Resolution{256};

  Calibration calib = Calibration(StrawMappingConstFile);
  ASSERT_EQ(calib.StrawCalibration.size(), Straws);
  ASSERT_EQ(calib.getMaxPixel(), Straws * Resolution);

  for (uint32_t Straw = 0; Straw < Straws; Straw++) {
    for (uint32_t Pos = 2; Pos < Resolution; Pos++) {

      ASSERT_EQ(calib.strawCorrection(Straw, Pos), Pos - Straw);
    }
  }
  deleteFile(StrawMappingConstFile);
}

TEST_F(CalibrationTest, NOTJson) {
  saveBuffer(NotJsonFile, (void *)NotJsonStr.c_str(), NotJsonStr.size());
  ASSERT_ANY_THROW(Calibration calib = Calibration(NotJsonFile));
  deleteFile(NotJsonFile);
}

TEST_F(CalibrationTest, BadStrawOrder) {
  saveBuffer(BadStrawOrderFile, (void *)BadStrawOrderStr.c_str(),
             BadStrawOrderStr.size());
  ASSERT_ANY_THROW(Calibration calib = Calibration(BadStrawOrderFile));
  deleteFile(BadStrawOrderFile);
}

TEST_F(CalibrationTest, StrawMismatch) {
  saveBuffer(StrawMismatchFile, (void *)StrawMismatchStr.c_str(),
             StrawMismatchStr.size());
  ASSERT_ANY_THROW(Calibration calib = Calibration(StrawMismatchFile));
  deleteFile(StrawMismatchFile);
}

TEST_F(CalibrationTest, InvalidCoeff) {
  saveBuffer(InvalidCoeffFile, (void *)InvalidCoeffStr.c_str(),
             InvalidCoeffStr.size());
  ASSERT_ANY_THROW(Calibration calib = Calibration(InvalidCoeffFile));
  deleteFile(InvalidCoeffFile);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto retval = RUN_ALL_TESTS();
  return retval;
}
