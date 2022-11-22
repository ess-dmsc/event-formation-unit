

// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Miracles position calculations
///
//===----------------------------------------------------------------------===//
#include <miracles/geometry/MiraclesGeometry.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

class MiraclesGeometryTest : public TestBase {
protected:
  MiraclesGeometry *geom;
  Config CaenConfiguration;
  void SetUp() override {
    geom = new MiraclesGeometry(CaenConfiguration);
    geom->NPos = 128;
  }
  void TearDown() override {}
};


TEST_F(MiraclesGeometryTest, Corner) {
  ASSERT_EQ(0, geom->xCoord(0, 0, 0, 5));
  ASSERT_EQ(0, geom->yCoord(0, 0, 5));

  ASSERT_EQ(47, geom->xCoord(1, 11, 5, 0));
  ASSERT_EQ(0, geom->yCoord(1, 5, 0));
  ASSERT_EQ(100, geom->yCoord(2, 5, 0));
}


TEST_F(MiraclesGeometryTest, ValidateData) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_TRUE(geom->validateData(readout));

  DataParser::CaenReadout readout2{11, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0};
  ASSERT_FALSE(geom->validateData(readout2));
}


TEST_F(MiraclesGeometryTest, CalcPixel) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);

  DataParser::CaenReadout readout2{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout2), 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
