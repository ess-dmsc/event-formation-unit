/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/ESSGeometry.h>
#include <test/TestBase.h>

class ESSGeometryTest : public TestBase {};

TEST_F(ESSGeometryTest, ConstructorValidParms) {
  ESSGeometry geom(5, 7, 11, 3);
  uint32_t max = 5 * 7 * 11 * 3;
  ASSERT_EQ(max, geom.getmaxpixel());
  ASSERT_EQ(1, geom.pixelSP2D(0, 0));
  ASSERT_EQ(1, geom.pixelMP2D(0, 0, 0));
  ASSERT_EQ(1, geom.pixelSP3D(0, 0, 0));
  ASSERT_EQ(1, geom.pixelMP3D(0, 0, 0, 0));
  ASSERT_EQ(max, geom.pixelMP3D(4, 6, 10, 2));
  ASSERT_EQ(1, geom.isvalidpixel(max));
  ASSERT_EQ(0, geom.isvalidpixel(max + 1));
  ASSERT_EQ(0, geom.isvalidpixel(-1));
}

TEST_F(ESSGeometryTest, ConstructorInvalidParms) {
  ESSGeometry sp_2d(5, 7, 0, 0);
  ASSERT_EQ(0, sp_2d.getmaxpixel());
  ASSERT_EQ(0, sp_2d.pixelSP2D(0, 0));
  ASSERT_EQ(0, sp_2d.pixelMP2D(0, 0, 0));
  ASSERT_EQ(0, sp_2d.pixelSP3D(0, 0, 0));
  ASSERT_EQ(0, sp_2d.pixelMP3D(0, 0, 0, 0));
  ASSERT_EQ(0, sp_2d.isvalidpixel(0));
  ASSERT_EQ(0, sp_2d.isvalidpixel(1));
}

TEST_F(ESSGeometryTest, SinglePanel2D) {
  uint32_t nx = 997;
  uint32_t ny = 991;
  ESSGeometry geom(nx, ny, 1, 1);
  ASSERT_EQ(nx * ny, geom.getmaxpixel());
  for (uint32_t y = 0; y < ny; y++)
    for (uint32_t x = 0; x < nx; x++) {
      ASSERT_EQ(y * nx + x + 1, geom.pixelSP2D(x, y));
    }
}

TEST_F(ESSGeometryTest, MultiPanel2D) {
  uint32_t nx = 997;
  uint32_t ny = 991;
  uint32_t np = 3;
  ESSGeometry geom(nx, ny, 1, np);
  ASSERT_EQ(nx * ny * np, geom.getmaxpixel());
  for (uint32_t p = 0; p < np; p++)
    for (uint32_t y = 0; y < ny; y++)
      for (uint32_t x = 0; x < nx; x++) {
        ASSERT_EQ(p * nx * ny + y * nx + x + 1, geom.pixelMP2D(x, y, p));
      }
}

TEST_F(ESSGeometryTest, SinglePanel3D) {
  uint32_t nx = 997;
  uint32_t ny = 991;
  uint32_t nz = 16;
  ESSGeometry geom(nx, ny, nz, 1);
  ASSERT_EQ(nx * ny * nz, geom.getmaxpixel());
  for (uint32_t z = 0; z < nz; z++)
    for (uint32_t y = 0; y < ny; y++)
      for (uint32_t x = 0; x < nx; x++) {
        ASSERT_EQ(z * nx * ny + y * nx + x + 1, geom.pixelSP3D(x, y, z));
      }
}

TEST_F(ESSGeometryTest, MultiPanel3D) {
  uint32_t nx = 397;
  uint32_t ny = 491;
  uint32_t nz = 16;
  uint32_t np = 3;
  ESSGeometry geom(nx, ny, nz, np);
  ASSERT_EQ(nx * ny * nz * np, geom.getmaxpixel());
  for (uint32_t p = 0; p < np; p++)
    for (uint32_t z = 0; z < nz; z++)
      for (uint32_t y = 0; y < ny; y++)
        for (uint32_t x = 0; x < nx; x++) {
          ASSERT_EQ(p * nx * ny * nz + z * nx * ny + y * nx + x + 1,
                    geom.pixelMP3D(x, y, z, p));
        }
}

TEST_F(ESSGeometryTest, GetCoordinates2x2x2x2) {
  ESSGeometry geom(2, 2, 2, 2);
  ASSERT_EQ(geom.xcoord(1), 0);
  ASSERT_EQ(geom.ycoord(1), 0);
  ASSERT_EQ(geom.zcoord(1), 0);
  ASSERT_EQ(geom.pcoord(1), 0);

  ASSERT_EQ(geom.xcoord(2), 1);
  ASSERT_EQ(geom.ycoord(2), 0);
  ASSERT_EQ(geom.zcoord(2), 0);
  ASSERT_EQ(geom.pcoord(2), 0);

  ASSERT_EQ(geom.xcoord(3), 0);
  ASSERT_EQ(geom.ycoord(3), 1);
  ASSERT_EQ(geom.zcoord(3), 0);
  ASSERT_EQ(geom.pcoord(3), 0);

  ASSERT_EQ(geom.xcoord(4), 1);
  ASSERT_EQ(geom.ycoord(4), 1);
  ASSERT_EQ(geom.zcoord(4), 0);
  ASSERT_EQ(geom.pcoord(4), 0);

  ASSERT_EQ(geom.xcoord(5), 0);
  ASSERT_EQ(geom.ycoord(5), 0);
  ASSERT_EQ(geom.zcoord(5), 1);
  ASSERT_EQ(geom.pcoord(5), 0);

  ASSERT_EQ(geom.xcoord(6), 1);
  ASSERT_EQ(geom.ycoord(6), 0);
  ASSERT_EQ(geom.zcoord(6), 1);
  ASSERT_EQ(geom.pcoord(6), 0);

  ASSERT_EQ(geom.xcoord(7), 0);
  ASSERT_EQ(geom.ycoord(7), 1);
  ASSERT_EQ(geom.zcoord(7), 1);
  ASSERT_EQ(geom.pcoord(7), 0);

  ASSERT_EQ(geom.xcoord(8), 1);
  ASSERT_EQ(geom.ycoord(8), 1);
  ASSERT_EQ(geom.zcoord(8), 1);
  ASSERT_EQ(geom.pcoord(8), 0);

  ASSERT_EQ(geom.xcoord(9), 0);
  ASSERT_EQ(geom.ycoord(9), 0);
  ASSERT_EQ(geom.zcoord(9), 0);
  ASSERT_EQ(geom.pcoord(9), 1);

  ASSERT_EQ(geom.xcoord(10), 1);
  ASSERT_EQ(geom.ycoord(10), 0);
  ASSERT_EQ(geom.zcoord(10), 0);
  ASSERT_EQ(geom.pcoord(10), 1);

  ASSERT_EQ(geom.xcoord(11), 0);
  ASSERT_EQ(geom.ycoord(11), 1);
  ASSERT_EQ(geom.zcoord(11), 0);
  ASSERT_EQ(geom.pcoord(11), 1);

  ASSERT_EQ(geom.xcoord(12), 1);
  ASSERT_EQ(geom.ycoord(12), 1);
  ASSERT_EQ(geom.zcoord(12), 0);
  ASSERT_EQ(geom.pcoord(12), 1);

  ASSERT_EQ(geom.xcoord(13), 0);
  ASSERT_EQ(geom.ycoord(13), 0);
  ASSERT_EQ(geom.zcoord(13), 1);
  ASSERT_EQ(geom.pcoord(13), 1);

  ASSERT_EQ(geom.xcoord(14), 1);
  ASSERT_EQ(geom.ycoord(14), 0);
  ASSERT_EQ(geom.zcoord(14), 1);
  ASSERT_EQ(geom.pcoord(14), 1);

  ASSERT_EQ(geom.xcoord(15), 0);
  ASSERT_EQ(geom.ycoord(15), 1);
  ASSERT_EQ(geom.zcoord(15), 1);
  ASSERT_EQ(geom.pcoord(15), 1);

  ASSERT_EQ(geom.xcoord(16), 1);
  ASSERT_EQ(geom.ycoord(16), 1);
  ASSERT_EQ(geom.zcoord(16), 1);
  ASSERT_EQ(geom.pcoord(16), 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
