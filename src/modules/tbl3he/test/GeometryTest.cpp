// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file

#include <tbl3he/geometry/Tbl3HeGeometry.h>
#include <common/testutils/TestBase.h>

using namespace Caen;



class Tbl3HeGeometryTest : public TestBase {
protected:
  Tbl3HeGeometry *geom{nullptr};
  Config CaenConfiguration;

  void SetUp() override {
    CaenConfiguration.Resolution = 100;
    CaenConfiguration.MaxRing = 1;
    CaenConfiguration.MaxFEN = 0;
    CaenConfiguration.MaxGroup = 7;
    geom = new Tbl3HeGeometry(CaenConfiguration);
  }
  void TearDown() override {}
};


TEST_F(Tbl3HeGeometryTest, Constructor) {
  ASSERT_EQ(geom->MaxRing, 1);
  ASSERT_EQ(geom->MaxFEN, 0);
  ASSERT_EQ(geom->MaxGroup, 7);
  ASSERT_EQ(geom->Stats.AmplitudeZero, 0);
  ASSERT_EQ(geom->Stats.RingErrors, 0);
  ASSERT_EQ(geom->Stats.FENErrors, 0);
  ASSERT_EQ(geom->Stats.GroupErrors, 0);
}


TEST_F(Tbl3HeGeometryTest, ValidateReadoutsFiberId) {
  geom = new Tbl3HeGeometry(CaenConfiguration);
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // Check valid FiberIds
  int MaxFiberId = (geom->MaxRing + 1)*2;
  ASSERT_EQ(MaxFiberId, 4);
  for (int i = 0; i <= 23; i++) {
    readout.FiberId = i;
    if (i < MaxFiberId) {
      ASSERT_EQ(geom->validateData(readout), true);
    } else {
      ASSERT_EQ(geom->validateData(readout), false);
    }
  }
  ASSERT_EQ(geom->Stats.RingErrors, 20);
  ASSERT_EQ(geom->Stats.FENErrors, 0);
  ASSERT_EQ(geom->Stats.GroupErrors, 0);
}


TEST_F(Tbl3HeGeometryTest, ValidateReadoutsFENId) {
  geom = new Tbl3HeGeometry(CaenConfiguration);
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // Check valid FENIds
  for (int i = 0; i <= 23; i++) {
    readout.FENId = i;
    if (i <= geom->MaxFEN) {
      ASSERT_EQ(geom->validateData(readout), true);
    } else {
      ASSERT_EQ(geom->validateData(readout), false);
    }
  }
  ASSERT_EQ(geom->Stats.RingErrors, 0);
  ASSERT_EQ(geom->Stats.FENErrors, 23);
  ASSERT_EQ(geom->Stats.GroupErrors, 0);
}


TEST_F(Tbl3HeGeometryTest, ValidateReadoutsGroup) {
  geom = new Tbl3HeGeometry(CaenConfiguration);
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // Check valid Groups
  for (int i = 0; i <= 23; i++) {
    readout.Group = i;
    if (i <= geom->MaxGroup) {
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
  geom = new Tbl3HeGeometry(CaenConfiguration);
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);
  ASSERT_EQ(geom->Stats.AmplitudeZero, 1);
}

TEST_F(Tbl3HeGeometryTest, CalcPixelOutOfRange) {
  geom = new Tbl3HeGeometry(CaenConfiguration);
  //                              R  F               G     A    B
  DataParser::CaenReadout readout{3, 0, 0, 0, 0, 0, 10, 0, 10, 10, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);
}


TEST_F(Tbl3HeGeometryTest, CalcPixelSelectedOK) {
  geom = new Tbl3HeGeometry(CaenConfiguration);
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


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
