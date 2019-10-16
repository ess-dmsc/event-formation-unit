/** Copyright (C) 2019 European Spallation Source ERIC */

#include <algorithm>
#include <memory>
#include <loki/geometry/Geometry.h>
#include <test/TestBase.h>

using namespace Loki;

class LokiGeometryTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(LokiGeometryTest, Constructor) {
  Geometry Geom(8, 4, 7, 512);
  ASSERT_EQ(Geom.getPixelId(28,  0, 511),      1); // valid tubeid
  ASSERT_EQ(Geom.getPixelId(32,  0, 511),      0); // invalid tubeid
  ASSERT_EQ(Geom.getPixelId(0,   6, 511),     56); // valid strawid
  ASSERT_EQ(Geom.getPixelId(28,  7, 511),      0); // invalid strawid
  ASSERT_EQ(Geom.getPixelId(3,   6, 511),  86072); // valid ypos
  ASSERT_EQ(Geom.getPixelId(3,   6, 512),      0); // invalid ypos
}

TEST_F(LokiGeometryTest, CornersZ0) {
  Geometry Geom(8, 4, 7, 512);
  ASSERT_EQ(Geom.getPixelId( 0,  6,   0),   28672); // bottom right
  ASSERT_EQ(Geom.getPixelId( 0,  6, 511),      56); // top right
  ASSERT_EQ(Geom.getPixelId(28,  0,   0),   28617); // bottom left
  ASSERT_EQ(Geom.getPixelId(28,  0, 511),       1); // top left
}

TEST_F(LokiGeometryTest, CornersZ1) {
  Geometry Geom(8, 4, 7, 512);
  auto offset = 8*7*512; // x dim: 8 * 7, y dim: 512
  ASSERT_EQ(Geom.getPixelId( 1,  6,   0),   28672 + offset); // bottom right
  ASSERT_EQ(Geom.getPixelId( 1,  6, 511),      56 + offset); // top right
  ASSERT_EQ(Geom.getPixelId(29,  0,   0),   28617 + offset); // bottom left
  ASSERT_EQ(Geom.getPixelId(29,  0, 511),       1 + offset); // top left
}

TEST_F(LokiGeometryTest, CornersZ2) {
  Geometry Geom(8, 4, 7, 512);
  auto offset = 2 * 8*7*512; // x dim: 8 * 7, y dim: 512
  ASSERT_EQ(Geom.getPixelId( 2,  6,   0),   28672 + offset); // bottom right
  ASSERT_EQ(Geom.getPixelId( 2,  6, 511),      56 + offset); // top right
  ASSERT_EQ(Geom.getPixelId(30,  0,   0),   28617 + offset); // bottom left
  ASSERT_EQ(Geom.getPixelId(30,  0, 511),       1 + offset); // top left
}

TEST_F(LokiGeometryTest, CornersZ3) {
  Geometry Geom(8, 4, 7, 512);
  auto offset = 3 * 8*7*512; // x dim: 8 * 7, y dim: 512
  ASSERT_EQ(Geom.getPixelId( 3,  6,   0),   28672 + offset); // bottom right
  ASSERT_EQ(Geom.getPixelId( 3,  6, 511),      56 + offset); // top right
  ASSERT_EQ(Geom.getPixelId(31,  0,   0),   28617 + offset); // bottom left
  ASSERT_EQ(Geom.getPixelId(31,  0, 511),       1 + offset); // top left
}
