// Copyright (C) 2019 - 2024 European Spallation Source, see LICENSE file, ERIC
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
    CaenConfiguration.LokiConf.Parms.Resolution = 512;
    CaenConfiguration.LokiConf.Parms.TotalGroups = 56*4;
    CaenConfiguration.LokiConf.Parms.Rings[0].Bank = 0;
    CaenConfiguration.LokiConf.Parms.Rings[0].FENs = 16;
    CaenConfiguration.LokiConf.Parms.Rings[0].FENOffset = 0;

    CaenConfiguration.LokiConf.Parms.Banks[0].GroupsN = 56;
    CaenConfiguration.LokiConf.Parms.Banks[0].YOffset = 0;
    geom = new LokiGeometry(CaenConfiguration);
    geom->setResolution(512);



    geom->CaenCDCalibration.Parms.Groups=CaenConfiguration.LokiConf.Parms.TotalGroups;
    // Make nullcalibration
    for (int i = 0; i < geom->Conf.LokiConf.Parms.TotalGroups; i++) {
      geom->CaenCDCalibration.Intervals.push_back({{0.0,0.143}, {0.144,0.286},
            {0.287,0.429}, {0.43,0.571}, {0.572,0.714},
            {0.715,0.857}, {0.858,  1.0}});
      geom->CaenCDCalibration.Calibration.push_back({{0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}});
    }
  }
  void TearDown() override {}
};

// Test cases below
TEST_F(LokiGeometryTest, AllZeroes) {
  geom->setResolution(512);
  geom->calcUnitAndPos(0, 0, 0, 0, 0);
  ASSERT_EQ(geom->Stats.AmplitudeZero, 1);
}


TEST_F(LokiGeometryTest, UnitLimits) {
  double delta = 0.0001;
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.1), 0);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.143 - delta), 0);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.143), 0);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.144 + delta), 1);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.286 - delta), 1);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.286), 1);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.287 + delta), 2);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.429 - delta), 2);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.429), 2);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.43 + delta), 3);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.571 - delta), 3);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.571), 3);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.572 + delta), 4);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.714 - delta), 4);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.714), 4);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.715 + delta), 5);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.857 - delta), 5);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.857), 5);
  ASSERT_EQ(geom->CaenCDCalibration.getUnitId(0, 0.858 + delta), 6);
}


///\brief Using formula from ICD to calculate Unit (straw in LOKI terms)
///
///            B + D
/// Unit = ------------- * 6
///        A + B + C + D
///
TEST_F(LokiGeometryTest, MinMaxUnit) {
  geom->setResolution(512);
  unsigned int iMax = 4096;
  for (unsigned int i = 1; i < iMax; i++) {
    //                           Group A  B  C  D
    auto Unit = geom->calcUnitAndPos(0, 0, i, 0, 0).first;
    ASSERT_EQ(Unit, 6);
    Unit = geom->calcUnitAndPos(0, 0, 0, 0, i).first;
    ASSERT_EQ(Unit, 6);
    Unit = geom->calcUnitAndPos(0, i, i, 0, 0).first;
    ASSERT_EQ(Unit, 3);
    Unit = geom->calcUnitAndPos(0, 0, 0, i, i).first;
    ASSERT_EQ(Unit, 3);
    Unit = geom->calcUnitAndPos(0, 0, 0, i, 0).first;
    ASSERT_EQ(Unit, 0);
  }
}


///\brief Using formula from ICD to calculate Position (in unit interval)
///
///            A + B
/// Pos  = -------------
///        A + B + C + D
///
TEST_F(LokiGeometryTest, MinMaxPos) {
  geom->setResolution(512);
  for (unsigned int i = 1; i < 4095; i++) {
    //                          Group  A  B  C  D
    auto Pos = geom->calcUnitAndPos(0, 0, 0, i, 0).second;
    ASSERT_EQ(Pos, 0);
    Pos = geom->calcUnitAndPos(0, 0, 0, 0, i).second;
    ASSERT_EQ(Pos, 0);
    Pos = geom->calcUnitAndPos(0, 0, 0, i, i).second;
    ASSERT_EQ(Pos, 0);
    Pos = geom->calcUnitAndPos(0, i, i, 0, 0).second;
    ASSERT_EQ(Pos, 1.0);
  }
}



TEST_F(LokiGeometryTest, validate) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_TRUE(geom->validateData(readout));

  readout.FiberId = 10;
  ASSERT_FALSE(geom->validateData(readout));
  ASSERT_EQ(geom->Stats.RingMappingErrors, 1);
  ASSERT_EQ(geom->Stats.FENErrors, 0);

  readout.FiberId = 0;
  readout.FENId = 20;
  ASSERT_FALSE(geom->validateData(readout));
  ASSERT_EQ(geom->Stats.RingMappingErrors, 1);
  ASSERT_EQ(geom->Stats.FENMappingErrors, 1);
}


///\todo Very rudimentary test, should be extended
TEST_F(LokiGeometryTest, calcPixel) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);

  readout.AmpA = 10;
  readout.AmpB = 10;
  readout.AmpC = 10;
  readout.AmpD = 10;
  // Should result in Unit = 3, Pos = 0.5
  // So pixel is 3 * 512 + 255 + 1 =
  ASSERT_EQ(geom->calcPixel(readout), 1793);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
