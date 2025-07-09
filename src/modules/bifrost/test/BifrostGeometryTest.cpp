// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file, ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Bifrost position calculations
///
//===----------------------------------------------------------------------===//
#include <bifrost/geometry/BifrostGeometry.h>
#include <caen/readout/DataParser.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

class BifrostGeometryTest : public TestBase {
protected:
  Config CaenConfiguration;
  BifrostGeometry *geom;

  int NullCalibGroup{0};
  int ManualCalibGroup{44};

  std::vector<std::pair<double, double>> NullCalib{
      {0.000, 0.333}, {0.334, 0.667}, {0.668, 1.000}};
  std::vector<std::pair<double, double>> ManualCalib{
      {0.030, 0.290}, {0.627, 0.363}, {0.705, 0.970}};

  void SetUp() override {
    geom = new BifrostGeometry(CaenConfiguration);
    geom->NPos = 300;
    geom->MaxRing = 4;

    CaenConfiguration.CaenParms.MaxGroup=45;
    geom->CaenCDCalibration.Parms.Groups=CaenConfiguration.CaenParms.MaxGroup;
    // Make nullcalibration
    for (int i = 0; i < 45; i++) {
      geom->CaenCDCalibration.Intervals.push_back(NullCalib);
      geom->CaenCDCalibration.Calibration.push_back(
          {{0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}});
    }
    geom->CaenCDCalibration.Intervals[ManualCalibGroup] = ManualCalib;

    // force the half-range upper limit on pulse-height values
    geom->MaxAmpl = 32767;
  }
  void TearDown() override {}
};

TEST_F(BifrostGeometryTest, YOffset) {
  ASSERT_EQ(geom->yOffset(0), 0);
  ASSERT_EQ(geom->yOffset(1), 0);
  ASSERT_EQ(geom->yOffset(2), 0);
  ASSERT_EQ(geom->yOffset(3), 3);
  ASSERT_EQ(geom->yOffset(4), 3);
  ASSERT_EQ(geom->yOffset(5), 3);
  ASSERT_EQ(geom->yOffset(6), 6);
  ASSERT_EQ(geom->yOffset(7), 6);
  ASSERT_EQ(geom->yOffset(8), 6);
  ASSERT_EQ(geom->yOffset(9), 9);
  ASSERT_EQ(geom->yOffset(10), 9);
  ASSERT_EQ(geom->yOffset(11), 9);
  ASSERT_EQ(geom->yOffset(12), 12);
  ASSERT_EQ(geom->yOffset(13), 12);
  ASSERT_EQ(geom->yOffset(14), 12);
}

TEST_F(BifrostGeometryTest, XOffset) {
  ASSERT_EQ(geom->xOffset(0, 0), 0);
  ASSERT_EQ(geom->xOffset(0, 1), 100);
  ASSERT_EQ(geom->xOffset(0, 2), 200);
  ASSERT_EQ(geom->xOffset(1, 0), 300);
  ASSERT_EQ(geom->xOffset(1, 1), 400);
  ASSERT_EQ(geom->xOffset(1, 2), 500);
  ASSERT_EQ(geom->xOffset(2, 0), 600);
  ASSERT_EQ(geom->xOffset(2, 1), 700);
  ASSERT_EQ(geom->xOffset(2, 2), 800);
}

TEST_F(BifrostGeometryTest, Position) {
  ASSERT_EQ(geom->calcUnitAndPos(0, 0, 0).first, -1);
  ASSERT_EQ(geom->calcUnitAndPos(0, 0, 1).second, 0.0);
  ASSERT_EQ(geom->calcUnitAndPos(0, 1, 0).second, 1.0);
}

TEST_F(BifrostGeometryTest, PosOutsideInterval) {
  // geom->CaenCalibration.BifrostCalibration.Calib =
  //       geom->CaenCalibration.BifrostCalibration.Intervals;
  ASSERT_EQ(geom->CaenCDCalibration.Stats.OutsideInterval, 0);
  std::pair<int, float> Result = geom->calcUnitAndPos(ManualCalibGroup, 100, 0);
  ASSERT_EQ(Result.first, -1);
  ASSERT_EQ(geom->CaenCDCalibration.Stats.OutsideInterval, 1);
}

TEST_F(BifrostGeometryTest, BadAmplitudes) {
  // A = -1, B = 20 -> pos < 0
  std::pair<int, float> Result = geom->calcUnitAndPos(ManualCalibGroup, -1, 20);
  ASSERT_EQ(Result.first, -1);
}

TEST_F(BifrostGeometryTest, TooLargeAmplitudes){
  // A + B > 32768  -> pos < 0
  auto Result = geom->calcUnitAndPos(ManualCalibGroup, 32767, 1);
  ASSERT_EQ(Result.first, -1);
  Result = geom->calcUnitAndPos(ManualCalibGroup, 1, 32767);
  ASSERT_EQ(Result.first, -1);
  Result = geom->calcUnitAndPos(ManualCalibGroup, 0, 32768);
  ASSERT_EQ(Result.first, -1);
  Result = geom->calcUnitAndPos(ManualCalibGroup, 32768, 0);
  ASSERT_EQ(Result.first, -1);
}

TEST_F(BifrostGeometryTest, MiddleUnit) {
  // 11/(11+9) > 0.5, middle tube swaps so pos should be < 0.5
  std::pair<int, float> Result = geom->calcUnitAndPos(ManualCalibGroup, 11, 9);
  ASSERT_EQ(Result.first, 1);
  ASSERT_TRUE(Result.second < 0.5);
}

TEST_F(BifrostGeometryTest, CalcPixel) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);

  DataParser::CaenReadout readout2{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout2), 1);
}

TEST_F(BifrostGeometryTest, Validate) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_TRUE(geom->validateData(readout));

  readout.FiberId = 10;
  ASSERT_FALSE(geom->validateData(readout));
  ASSERT_EQ(geom->Stats.RingErrors, 1);
  ASSERT_EQ(geom->Stats.FENErrors, 0);
  ASSERT_EQ(geom->Stats.GroupErrors, 0);

  readout.FiberId = 0;
  readout.FENId = 20;
  ASSERT_FALSE(geom->validateData(readout));
  ASSERT_EQ(geom->Stats.RingErrors, 1);
  ASSERT_EQ(geom->Stats.FENErrors, 1);
  ASSERT_EQ(geom->Stats.GroupErrors, 0);

  readout.FiberId = 0;
  readout.FENId = 0;
  readout.Group = 20;
  ASSERT_FALSE(geom->validateData(readout));
  ASSERT_EQ(geom->Stats.RingErrors, 1);
  ASSERT_EQ(geom->Stats.FENErrors, 1);
  ASSERT_EQ(geom->Stats.GroupErrors, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
