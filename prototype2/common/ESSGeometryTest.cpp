/** Copyright (C) 2018 European Spallation Source */

#include <common/ESSGeometry.h>
#include <test/TestBase.h>


class ESSGeometryTest : public TestBase {};

TEST_F(ESSGeometryTest, Constructor) {
  ESSGeometry essgeom(1,1,1,1);

  ASSERT_EQ(0, essgeom.getPixelSP2D(1, 1));
  ASSERT_EQ(0, essgeom.getPixelSP2D(0, 1));
  ASSERT_EQ(0, essgeom.getPixelSP2D(1, 0));
  ASSERT_EQ(1, essgeom.getPixelSP2D(0, 0));

  ASSERT_EQ(0, essgeom.getPixelSP3D(1, 1, 1));
  ASSERT_EQ(0, essgeom.getPixelSP3D(1, 1, 0));
  ASSERT_EQ(0, essgeom.getPixelSP3D(1, 0, 1));
  ASSERT_EQ(0, essgeom.getPixelSP3D(1, 0, 0));
  ASSERT_EQ(0, essgeom.getPixelSP3D(0, 1, 1));
  ASSERT_EQ(0, essgeom.getPixelSP3D(0, 1, 0));
  ASSERT_EQ(0, essgeom.getPixelSP3D(0, 0, 1));
  ASSERT_EQ(1, essgeom.getPixelSP3D(0, 0, 0));
}

TEST_F(ESSGeometryTest, ConstructorII) {
  size_t nx = 5;
  size_t ny = 7;
  size_t nz = 11;
  size_t np = 3;
  ESSGeometry essgeom(5,7,11,3);

  ASSERT_EQ(nx * ny * nz * np, essgeom.getPixelMP3D(nx - 1, ny - 1, nz - 1, np - 1));

  ASSERT_EQ(nx * ny * np, essgeom.getPixelMP2D(nx - 1, ny - 1, np - 1));

  ASSERT_EQ(nx * ny * nz, essgeom.getPixelSP3D(nx - 1, ny - 1, nz - 1));

  ASSERT_EQ(nx * ny, essgeom.getPixelSP2D(nx - 1, ny - 1));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
