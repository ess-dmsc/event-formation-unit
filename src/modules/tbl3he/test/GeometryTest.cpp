// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <tbl3he/geometry/Tbl3HeGeometry.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

class Tbl3HeGeometryTest : public TestBase {
protected:
  Tbl3HeGeometry *geom{nullptr};
  Config CaenConfiguration;

  void SetUp() override {
    CaenConfiguration.Tbl3HeConf.Parms.Resolution = 100;
    CaenConfiguration.Tbl3HeConf.Parms.MaxGroup = 7;
    geom = new Tbl3HeGeometry(CaenConfiguration);

    geom->CaenCDCalibration.Parms.Groups=8;
    // Make nullcalibration for the eight groups
    for (int i = 0; i < 8; i++) {
      geom->CaenCDCalibration.Intervals.push_back({{0.00, 1.00}});
      geom->CaenCDCalibration.Calibration.push_back({{0.0, 0.0, 0.0, 0.0}});
    }

    geom->Conf.TopologyMapPtr.reset(new HashMap2D<Caen::Tbl3HeConfig::Topology>(2));
    auto topo = std::make_unique<Caen::Tbl3HeConfig::Topology>(0);
    geom->Conf.TopologyMapPtr->add(0, 0, topo);
    topo = std::make_unique<Caen::Tbl3HeConfig::Topology>(1);
    geom->Conf.TopologyMapPtr->add(1, 0, topo);
  }
  void TearDown() override {}
};


TEST_F(Tbl3HeGeometryTest, Constructor) {
  ASSERT_EQ(geom->MaxFEN, 0);
  ASSERT_EQ(geom->Stats.AmplitudeZero, 0);
  ASSERT_EQ(geom->Stats.RingErrors, 0);
  ASSERT_EQ(geom->Stats.FENErrors, 0);
  ASSERT_EQ(geom->Stats.GroupErrors, 0);
}


TEST_F(Tbl3HeGeometryTest, ValidateReadouts) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // Check valid FiberIds
  for (int FiberId = 0; FiberId < 24; FiberId++) {
    for (int FENId = 0; FENId < 12; FENId++) {
        readout.FiberId = FiberId;
        readout.FENId = FENId;
        if ((FiberId < 4) and (FENId < 1)) {
          ASSERT_EQ(geom->validateData(readout), true);
        } else {
          ASSERT_EQ(geom->validateData(readout), false);
        }
      }
    }
  ASSERT_EQ(geom->Stats.TopologyErrors, 24 * 12 - 4);
  ASSERT_EQ(geom->Stats.GroupErrors, 0);
}


TEST_F(Tbl3HeGeometryTest, ValidateReadoutsGroup) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // Check valid Groups
  for (int i = 0; i <= 23; i++) {
    readout.Group = i;
    if (i <= geom->Conf.Parms.MaxGroup) {
      ASSERT_EQ(geom->validateData(readout), true);
    } else {
      ASSERT_EQ(geom->validateData(readout), false);
    }
  }
  ASSERT_EQ(geom->Stats.RingErrors, 0);
  ASSERT_EQ(geom->Stats.FENErrors, 0);
  ASSERT_EQ(geom->Stats.GroupErrors, 16);
}


TEST_F(Tbl3HeGeometryTest, CalcPixelBadAmpl) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);
  ASSERT_EQ(geom->Stats.AmplitudeZero, 1);
}


TEST_F(Tbl3HeGeometryTest, CalcPixelOutOfRange) {
  //                              R  F               G     A    B
  DataParser::CaenReadout readout{3, 0, 0, 0, 0, 0, 10, 0, 10, 10, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);
}


TEST_F(Tbl3HeGeometryTest, CalcPixelSelectedOK) {
  //                               R  F              G     A   B  C  D
  DataParser::CaenReadout readout1{0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout1), 1);

  DataParser::CaenReadout readout2{0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout2), 100);
  //                               R  F              G      A  B  C  D
  DataParser::CaenReadout readout3{1, 0, 0, 0, 0, 0, 7, 0,  0, 10, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout3), 701);

  DataParser::CaenReadout readout4{1, 0, 0, 0, 0, 0, 7, 0, 10, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout4), 800);
}


TEST_F(Tbl3HeGeometryTest, OutsideUnitInterval) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 10, -1, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);
}


TEST_F(Tbl3HeGeometryTest, Serialisers) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0};
  ASSERT_EQ(geom->calcSerializer(readout), 0);
  readout.FiberId = 1;
  ASSERT_EQ(geom->calcSerializer(readout), 0);
  readout.FiberId = 2;
  ASSERT_EQ(geom->calcSerializer(readout), 1);
  readout.FiberId = 3;
  ASSERT_EQ(geom->calcSerializer(readout), 1);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
