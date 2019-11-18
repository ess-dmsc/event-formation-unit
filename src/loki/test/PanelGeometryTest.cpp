/** Copyright (C) 2019 European Spallation Source ERIC */

#include <algorithm>
#include <memory>
#include <loki/geometry/PanelGeometry.h>
#include <test/TestBase.h>

using namespace Loki;

class PanelGeometryTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(PanelGeometryTest, Constructor) {
  PanelGeometry PG(true, 4, 8, 0);
  const uint8_t FEN{3};
  const uint8_t FPGA{3};
  const uint8_t Tube{4};
  const uint8_t Straw{0};
  const uint16_t POS{511};
  uint32_t Pixel = PG.getPixel2D(FEN, FPGA, Tube, Straw, POS);
  ASSERT_EQ(Pixel, 1);
}

TEST_F(PanelGeometryTest, CanonicalCorners) {
  PanelGeometry PG(true, 4, 8, 0);
  ASSERT_EQ(PG.getPixel2D(3, 3, 4, 0, 511), 1); // Top Left
  ASSERT_EQ(PG.getPixel2D(0, 0, 3, 6, 511), 8*7*4); // Top Right
  ASSERT_EQ(PG.getPixel2D(3, 3, 4, 0, 0), 114465); // Bottom Left
  ASSERT_EQ(PG.getPixel2D(0, 0, 3, 6, 0),  (8*7*4)*512); // Bottom Right
}

TEST_F(PanelGeometryTest, CanonicalCornersRotated) {
  PanelGeometry PG(false, 4, 8, 0);
  ASSERT_EQ(PG.getPixel2D(0, 0, 3, 6, 511), 1); // Top Left
  ASSERT_EQ(PG.getPixel2D(0, 0, 3, 6,   0), 512); // Top Right
  ASSERT_EQ(PG.getPixel2D(3, 3, 4, 0, 511), (8*7*4 - 1)*512 + 1); // Bottom Left
  ASSERT_EQ(PG.getPixel2D(3, 3, 4, 0,   0), (8*7*4)*512); // Bottom Right
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
