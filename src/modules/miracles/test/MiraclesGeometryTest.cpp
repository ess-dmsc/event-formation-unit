

// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Miracles position calculations
///
//===----------------------------------------------------------------------===//
#include <common/testutils/TestBase.h>
#include <miracles/geometry/MiraclesGeometry.h>

using namespace Caen;

class MiraclesGeometryTest : public TestBase {
protected:
  int TubeA{0};
  int TubeB{1};
  MiraclesGeometry *geom;
  Config CaenConfiguration;
  void SetUp() override {
    geom = new MiraclesGeometry(CaenConfiguration);
    geom->NPos = 128;
    geom->MaxRing = 2; // Miracles has 3 rings, but we use 0-2 for 3 rings
  }
  void TearDown() override {}
};

TEST_F(MiraclesGeometryTest, Corner) {
  ASSERT_EQ(1, geom->xCoord(0, 0, 0, 5));
  ASSERT_EQ(0, geom->yCoord(0, 0, 5));

  ASSERT_EQ(46, geom->xCoord(1, 11, 5, 0));
  ASSERT_EQ(0, geom->yCoord(1, 5, 0));
  ASSERT_EQ(64, geom->yCoord(2, 5, 0));
}

TEST_F(MiraclesGeometryTest, PosAlongTube) {
  printf("B top\n");
  ASSERT_EQ(geom->tubeAorB(0, 1), TubeB); // 1 is B
  ASSERT_EQ(geom->posAlongUnit(0, 1), 0); // tube B - top 'pixel'
  printf("A bottom\n");
  ASSERT_EQ(geom->tubeAorB(1, 1), TubeA);  // 0 is A
  ASSERT_EQ(geom->posAlongUnit(1, 1), 63); // tube A - bottom 'pixel'
  printf("A bottom\n");
  EXPECT_EQ(geom->tubeAorB(101, 100), TubeA);  // 0 is A
  EXPECT_EQ(geom->posAlongUnit(101, 100), 63); // tube A - bottom 'pixel'
  printf("A top\n");
  EXPECT_EQ(geom->tubeAorB(800, 1), TubeA); // 0 is A
  EXPECT_EQ(geom->posAlongUnit(800, 1), 0); // tube A - top 'pixel'
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
  ASSERT_EQ(geom->calcPixel(readout2), 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
