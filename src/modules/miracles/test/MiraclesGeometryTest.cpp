

// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Miracles position calculations
///
//===----------------------------------------------------------------------===//
#include "common/Statistics.h"
#include <common/testutils/TestBase.h>
#include <memory>
#include <miracles/geometry/MiraclesGeometry.h>

using namespace Caen;

class MiraclesGeometryTest : public TestBase {
protected:
  int TubeA{0};
  int TubeB{1};
  Statistics Stats;
  std::unique_ptr<MiraclesGeometry> geom;
  Config CaenConfiguration;

  void SetUp() override {
    CaenConfiguration.CaenParms.MaxRing = 2; // Miracles has 3 rings, but we use 0-2 for 3 rings
    CaenConfiguration.CaenParms.Resolution = 128;
    geom = std::make_unique<MiraclesGeometry>(Stats, CaenConfiguration);
  }

  void TearDown() override {}
};

TEST_F(MiraclesGeometryTest, Corner) {
  ASSERT_EQ(0, geom->xCoord(0, 0, 0, 5));
  ASSERT_EQ(0, geom->yCoord(0, 0, 5));

  ASSERT_EQ(47, geom->xCoord(1, 11, 5, 0));
  ASSERT_EQ(0, geom->yCoord(1, 5, 0));
  ASSERT_EQ(64, geom->yCoord(2, 5, 0));
}

TEST_F(MiraclesGeometryTest, PosAlongTube) {
  printf("A top\n");
  ASSERT_EQ(geom->tubeAorB(0, 1), TubeA); // 0 ia A
  ASSERT_EQ(geom->posAlongUnit(0, 1), 0); // tube A - top 'pixel'
  printf("A bottom\n");
  ASSERT_EQ(geom->tubeAorB(1, 1), TubeA);  // 0 ia A
  ASSERT_EQ(geom->posAlongUnit(1, 1), 63); // tube A - bottom 'pixel'
  printf("B bottom\n");
  EXPECT_EQ(geom->tubeAorB(101, 100), TubeB);  // 1 ia B
  EXPECT_EQ(geom->posAlongUnit(101, 100), 63); // tube B - bottom 'pixel'
  printf("B top\n");
  EXPECT_EQ(geom->tubeAorB(800, 1), TubeB); // 1 ia B
  EXPECT_EQ(geom->posAlongUnit(800, 1), 0); // tube B - top 'pixel'
}

TEST_F(MiraclesGeometryTest, ValidateData) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_TRUE(geom->validateReadoutData(readout));

  DataParser::CaenReadout readout2{11, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0};
  ASSERT_FALSE(geom->validateReadoutData(readout2));
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
