

// Copyright (C) 2022 European Spallation Source, see LICENSE file
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
  int64_t FENErrors, RingErrors, TubeErrors;
  void SetUp() override {
    geom = new MiraclesGeometry(CaenConfiguration);
    geom->NPos = 128;
    geom->Stats.FENErrors = &FENErrors;
    geom->Stats.RingErrors = &RingErrors;
    geom->Stats.TubeErrors = &TubeErrors;
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
  ASSERT_EQ(geom->posAlongTube(0, 1),  0); // tube A - top 'pixel'
  printf("A bottom\n");
  ASSERT_EQ(geom->tubeAorB(1, 1), TubeA); // 0 ia A
  ASSERT_EQ(geom->posAlongTube(1, 1), 63); // tube A - bottom 'pixel'
  printf("B bottom\n");
  EXPECT_EQ(geom->tubeAorB(101, 100), TubeB); // 1 ia B
  EXPECT_EQ(geom->posAlongTube(101, 100),  63); // tube B - bottom 'pixel'
  printf("B top\n");
  EXPECT_EQ(geom->tubeAorB(800, 1), TubeB); // 1 ia B
  EXPECT_EQ(geom->posAlongTube(800, 1),  0); // tube B - top 'pixel'
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
