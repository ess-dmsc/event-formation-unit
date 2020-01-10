/** Copyright (C) 2019 European Spallation Source ERIC */

#include <algorithm>
#include <memory>
#include <loki/geometry/PanelGeometry.h>
#include <test/TestBase.h>

using namespace Loki;

class PanelGeometryTest : public TestBase {
protected:

  const bool Vertical{true};
  const bool Horizontal{false};
  const uint32_t Offset0{0};
  const uint16_t TubesXorY8{8}; ///< # tubes in x/y direction
  const uint16_t TZ4{4}; ///< # tubes in z-direction
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(PanelGeometryTest, Constructor) {
  PanelGeometry PG(Vertical, TZ4, TubesXorY8, Offset0);
  const uint8_t FEN{3};
  const uint8_t FPGA{3};
  const uint8_t Tube{4};
  const uint8_t Straw{0};
  const uint16_t POS{511};
  uint32_t Pixel = PG.getPixel2D(FEN, FPGA, Tube, Straw, POS);
  ASSERT_EQ(Pixel, 1);
}

TEST_F(PanelGeometryTest, CanonicalCorners) {
  PanelGeometry PG(Vertical, TZ4, TubesXorY8, Offset0);
  ASSERT_EQ(PG.getPixel2D(3, 3, 4, 0, 511), 1); // Top Left
  ASSERT_EQ(PG.getPixel2D(0, 0, 3, 6, 511), 8*7*4); // Top Right
  ASSERT_EQ(PG.getPixel2D(3, 3, 4, 0,   0), 114465); // Bottom Left
  ASSERT_EQ(PG.getPixel2D(0, 0, 3, 6,   0),  (8*7*4)*512); // Bottom Right
}

TEST_F(PanelGeometryTest, CanonicalCornersWithOffset) {
  const uint32_t Offset{10000};
  PanelGeometry PG(Vertical, TZ4, TubesXorY8, Offset);
  ASSERT_EQ(PG.getPixel2D(3, 3, 4, 0, 511), 1 + Offset); // Top Left
  ASSERT_EQ(PG.getPixel2D(0, 0, 3, 6, 511), 8*7*4 + Offset); // Top Right
  ASSERT_EQ(PG.getPixel2D(3, 3, 4, 0, 0), 114465 + Offset); // Bottom Left
  ASSERT_EQ(PG.getPixel2D(0, 0, 3, 6, 0),  (8*7*4)*512 + Offset); // Bottom Right
}

TEST_F(PanelGeometryTest, CanonicalCornersRotated) {
  PanelGeometry PG(Horizontal, TZ4, TubesXorY8, Offset0);
  ASSERT_EQ(PG.getPixel2D(0, 0, 3, 6, 511), 1); // Top Left
  ASSERT_EQ(PG.getPixel2D(0, 0, 3, 6,   0), 512); // Top Right
  ASSERT_EQ(PG.getPixel2D(3, 3, 4, 0, 511), (8*7*4 - 1)*512 + 1); // Bottom Left
  ASSERT_EQ(PG.getPixel2D(3, 3, 4, 0,   0), (8*7*4)*512); // Bottom Right
}

TEST_F(PanelGeometryTest, InvalidIndices3D) {
  PanelGeometry PG(Vertical, TZ4, TubesXorY8, Offset0); // 4*8==32 tubes in total
  ASSERT_GT(PG.getPixel3D(0, 0, 7, 0, 511),      0); // max valid tubeid
  ASSERT_EQ(PG.getPixel3D(0, 0, 8, 0, 511),      0); // invalid tubeid
  ASSERT_GT(PG.getPixel3D(0, 0, 7, 6, 511),      0); // max valid strawid
  ASSERT_EQ(PG.getPixel3D(0, 0, 0, 7, 511),      0); // invalid strawid
  ASSERT_GT(PG.getPixel3D(0, 0, 0, 0, 511),      0); // max valid ypos
  ASSERT_EQ(PG.getPixel3D(0, 0, 0, 0, 512),      0); // invalid ypos
  ASSERT_EQ(PG.getPixel3D(0, 4, 0, 0, 511),      0); // 'invalid' FPGAId (for geometry)
}

TEST_F(PanelGeometryTest, InvalidIndices2D) {
  PanelGeometry PG(Vertical, TZ4, TubesXorY8, Offset0); // 4*8==32 tubes in total
  ASSERT_GT(PG.getPixel2D(0, 0, 7, 0, 511),      0); // max valid tubeid
  ASSERT_EQ(PG.getPixel2D(0, 0, 8, 0, 511),      0); // invalid tubeid
  ASSERT_GT(PG.getPixel2D(0, 0, 7, 6, 511),      0); // max valid strawid
  ASSERT_EQ(PG.getPixel2D(0, 0, 0, 7, 511),      0); // invalid strawid
  ASSERT_GT(PG.getPixel2D(0, 0, 0, 0, 511),      0); // max valid ypos
  ASSERT_EQ(PG.getPixel2D(0, 0, 0, 0, 512),      0); // invalid ypos
  ASSERT_EQ(PG.getPixel2D(0, 4, 0, 0, 511),      0); // 'invalid' FPGAId (for geometry)
}

TEST_F(PanelGeometryTest, CornersZ0) {
  PanelGeometry PG(Vertical, TZ4, TubesXorY8, Offset0);
  ASSERT_EQ(PG.getPixel3D( 0, 0, 0, 6,   0),   28672); // bottom right
  ASSERT_EQ(PG.getPixel3D( 0, 0, 0, 6, 511),      56); // top right
  ASSERT_EQ(PG.getPixel3D( 3, 3, 4, 0,   0),   28617); // bottom left
  ASSERT_EQ(PG.getPixel3D( 3, 3, 4, 0, 511),       1); // top left
}

TEST_F(PanelGeometryTest, CornersZ1) {
  PanelGeometry PG(Vertical, TZ4, TubesXorY8, Offset0);
  auto offset = 8*7*512; // x dim: 8 * 7, y dim: 512
  ASSERT_EQ(PG.getPixel3D( 0, 0, 1, 6,   0),   28672 + offset); // bottom right
  ASSERT_EQ(PG.getPixel3D( 0, 0, 1, 6, 511),      56 + offset); // top right
  ASSERT_EQ(PG.getPixel3D( 3, 3, 5, 0,   0),   28617 + offset); // bottom left
  ASSERT_EQ(PG.getPixel3D( 3, 3, 5, 0, 511),       1 + offset); // top left
}

TEST_F(PanelGeometryTest, CornersZ2) {
  PanelGeometry PG(Vertical, TZ4, TubesXorY8, Offset0);
  auto offset = 2 * 8*7*512; // x dim: 8 * 7, y dim: 512
  ASSERT_EQ(PG.getPixel3D( 0, 0, 2, 6,   0),   28672 + offset); // bottom right
  ASSERT_EQ(PG.getPixel3D( 0, 0, 2, 6, 511),      56 + offset); // top right
  ASSERT_EQ(PG.getPixel3D( 3, 3, 6, 0,   0),   28617 + offset); // bottom left
  ASSERT_EQ(PG.getPixel3D( 3, 3, 6, 0, 511),       1 + offset); // top left
}

TEST_F(PanelGeometryTest, CornersZ3) {
  PanelGeometry PG(Vertical, TZ4, TubesXorY8, Offset0);
  auto offset = 3 * 8*7*512; // x dim: 8 * 7, y dim: 512
  ASSERT_EQ(PG.getPixel3D( 0, 0, 3, 6,   0),   28672 + offset); // bottom right
  ASSERT_EQ(PG.getPixel3D( 0, 0, 3, 6, 511),      56 + offset); // top right
  ASSERT_EQ(PG.getPixel3D( 3, 3, 7, 0,   0),   28617 + offset); // bottom left
  ASSERT_EQ(PG.getPixel3D( 3, 3, 7, 0, 511),       1 + offset); // top left
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
