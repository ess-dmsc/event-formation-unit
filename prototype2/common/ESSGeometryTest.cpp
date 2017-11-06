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
  ESSGeometry sp_2d(5,7, 0, 0);
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
  ASSERT_EQ(nx*ny, geom.getmaxpixel());
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
  ASSERT_EQ(nx*ny*np, geom.getmaxpixel());
  for (uint32_t p = 0; p < np; p++)
    for (uint32_t y = 0; y < ny; y++)
      for (uint32_t x = 0; x < nx; x++) {
        ASSERT_EQ(p* nx*ny + y * nx + x + 1, geom.pixelMP2D(x, y, p));
      }
}

TEST_F(ESSGeometryTest, SinglePanel3D) {
  uint32_t nx = 997;
  uint32_t ny = 991;
  uint32_t nz = 16;
  ESSGeometry geom(nx, ny, nz, 1);
  ASSERT_EQ(nx*ny*nz, geom.getmaxpixel());
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
  ASSERT_EQ(nx*ny*nz*np, geom.getmaxpixel());
  for (uint32_t p = 0; p < np; p++)
    for (uint32_t z = 0; z < nz; z++)
      for (uint32_t y = 0; y < ny; y++)
        for (uint32_t x = 0; x < nx; x++) {
          ASSERT_EQ(p * nx * ny * nz + z * nx * ny + y * nx + x + 1, geom.pixelMP3D(x, y, z, p));
        }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
