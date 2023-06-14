// Copyright (C) 2019 - 2023 European Spallation Source, see LICENSE file, ERIC
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for LokiGeometry class
///
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <common/testutils/TestBase.h>
#include <loki/geometry/LokiGeometry.h>
#include <memory>

using namespace Caen;

class LokiGeometryTest : public TestBase {
protected:
  LokiGeometry *geom;
  Config CaenConfiguration;
  void SetUp() override {
    PanelGeometry Panel(4, 7, 0);
    CaenConfiguration.Panels.push_back(Panel);
    CaenConfiguration.Resolution = 512;
    CaenConfiguration.NTubesTotal = 28;
    geom = new LokiGeometry(CaenConfiguration);
    geom->setResolution(512);

    // Make nullcalibration
    for (int i = 0; i < CaenConfiguration.NTubesTotal; i++) {
      geom->CaenCDCalibration.Intervals.push_back({{0.0,0.143}, {0.144,0.286}, {0.287,0.429}, {0.43,0.571}, {0.572,0.714}, {0.715,0.857}, {0.858,  1.0}});
      geom->CaenCDCalibration.Calibration.push_back({{0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}});
    }
  }
  void TearDown() override {}
};

// Test cases below
TEST_F(LokiGeometryTest, Constructor) {
  geom->setResolution(512);
  ASSERT_EQ(geom->StrawId, 7);  // valid: 0 - 6
  ASSERT_EQ(geom->PosVal, 1); // valid: 0.0 - 1.0
}

TEST_F(LokiGeometryTest, AllZeroes) {
  geom->setResolution(512);
  geom->calcPositions(0, 0, 0, 0);
  ASSERT_EQ(geom->StrawId, 7);  // valid: 0 - 6
  ASSERT_EQ(geom->PosVal, 512); // valid: 0 - 511
  ASSERT_EQ(geom->Stats.AmplitudeZero, 1);
}

TEST_F(LokiGeometryTest, StrawLimits) {
  double delta = 0.0001;
  ASSERT_EQ(geom->strawCalc(0.1), 0);
  ASSERT_EQ(geom->strawCalc(0.7 - delta), 0);
  ASSERT_EQ(geom->strawCalc(0.7), 0);
  ASSERT_EQ(geom->strawCalc(0.7 + delta), 1);
  ASSERT_EQ(geom->strawCalc(1.56 - delta), 1);
  ASSERT_EQ(geom->strawCalc(1.56), 1);
  ASSERT_EQ(geom->strawCalc(1.56 + delta), 2);
  ASSERT_EQ(geom->strawCalc(2.52 - delta), 2);
  ASSERT_EQ(geom->strawCalc(2.52), 2);
  ASSERT_EQ(geom->strawCalc(2.52 + delta), 3);
  ASSERT_EQ(geom->strawCalc(3.54 - delta), 3);
  ASSERT_EQ(geom->strawCalc(3.54), 3);
  ASSERT_EQ(geom->strawCalc(3.54 + delta), 4);
  ASSERT_EQ(geom->strawCalc(4.44 - delta), 4);
  ASSERT_EQ(geom->strawCalc(4.44), 4);
  ASSERT_EQ(geom->strawCalc(4.44 + delta), 5);
  ASSERT_EQ(geom->strawCalc(5.30 - delta), 5);
  ASSERT_EQ(geom->strawCalc(5.30), 5);
  ASSERT_EQ(geom->strawCalc(5.30 + delta), 6);
}

TEST_F(LokiGeometryTest, MinMaxStraw) {
  geom->setResolution(512);
  unsigned int iMax = 4096;
  for (unsigned int i = 1; i < iMax; i++) {
    geom->calcPositions(0, i, 0, 0);
    ASSERT_EQ(geom->StrawId, 6);
    geom->calcPositions(0, 0, 0, i);
    ASSERT_EQ(geom->StrawId, 6);
    geom->calcPositions(0, i, 0, i);
    ASSERT_EQ(geom->StrawId, 6);

    geom->calcPositions(i, 0, i, 0);
    ASSERT_EQ(geom->StrawId, 0);
  }
}

TEST_F(LokiGeometryTest, MinMaxPos) {
  geom->setResolution(512);
  for (unsigned int i = 1; i < 4095; i++) {
    geom->calcPositions(0, 0, i, 0);
    ASSERT_EQ(geom->PosVal, 0);
    geom->calcPositions(0, 0, 0, i);
    ASSERT_EQ(geom->PosVal, 0);
    geom->calcPositions(0, 0, i, i);
    ASSERT_EQ(geom->PosVal, 0);

    geom->calcPositions(i, i, 0, 0);
    ASSERT_EQ(geom->PosVal, 1.0);
  }
}

TEST_F(LokiGeometryTest, validate) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_TRUE(geom->validateData(readout));

  readout.RingId = 10;
  ASSERT_FALSE(geom->validateData(readout));
  ASSERT_EQ(geom->Stats.RingErrors, 1);
  ASSERT_EQ(geom->Stats.FENErrors, 0);

  readout.RingId = 0;
  readout.FENId = 20;
  ASSERT_FALSE(geom->validateData(readout));
  ASSERT_EQ(geom->Stats.RingErrors, 1);
  ASSERT_EQ(geom->Stats.FENErrors, 1);
}

TEST_F(LokiGeometryTest, calcPixel) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);

  readout.AmpA = 10;
  readout.AmpB = 10;
  readout.AmpC = 10;
  readout.AmpD = 10;
  ASSERT_EQ(geom->calcPixel(readout), 1792);

  readout.FENId = 30;
  ASSERT_EQ(geom->calcPixel(readout), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
