/** Copyright (C) 2018 European Spallation Source */

#include <logical_geometry/ESSGeometry.h>
#include <test/TestBase.h>


class ESSGeometryTest : public TestBase {};

TEST_F(ESSGeometryTest, Constructor) {
  ESSGeometry essgeom(1,1,1,1);

  ASSERT_EQ(0, essgeom.pixel2D(1, 1));
  ASSERT_EQ(0, essgeom.pixel2D(0, 1));
  ASSERT_EQ(0, essgeom.pixel2D(1, 0));
  ASSERT_EQ(1, essgeom.pixel2D(0, 0));

  ASSERT_EQ(0, essgeom.pixel3D(1, 1, 1));
  ASSERT_EQ(0, essgeom.pixel3D(1, 1, 0));
  ASSERT_EQ(0, essgeom.pixel3D(1, 0, 1));
  ASSERT_EQ(0, essgeom.pixel3D(1, 0, 0));
  ASSERT_EQ(0, essgeom.pixel3D(0, 1, 1));
  ASSERT_EQ(0, essgeom.pixel3D(0, 1, 0));
  ASSERT_EQ(0, essgeom.pixel3D(0, 0, 1));
  ASSERT_EQ(1, essgeom.pixel3D(0, 0, 0));
}

TEST_F(ESSGeometryTest, ConstructorII) {
  size_t nx = 5;
  size_t ny = 7;
  size_t nz = 11;
  size_t np = 3;
  ESSGeometry essgeom(5,7,11,3);

  ASSERT_EQ(nx * ny * nz * np, essgeom.pixelMP3D(nx - 1, ny - 1, nz - 1, np - 1));

  essgeom.nz(1);
  ASSERT_EQ(nx * ny * np, essgeom.pixelMP2D(nx - 1, ny - 1, np - 1));

  essgeom.np(1);
  essgeom.nz(nz);
  ASSERT_EQ(nx * ny * nz, essgeom.pixel3D(nx - 1, ny - 1, nz - 1));

  essgeom.nz(1);
  ASSERT_EQ(nx * ny, essgeom.pixel2D(nx - 1, ny - 1));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
