/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/ESSGeometry.h>
#include <test/TestBase.h>

class ESSGeometryTest : public TestBase {};

TEST_F(ESSGeometryTest, DefaultConstructor) {
  ESSGeometry geom;
  ASSERT_FALSE(geom.valid());
}

TEST_F(ESSGeometryTest, ConstructorValidParms) {
  ESSGeometry geom(5, 7, 11, 3);
  ASSERT_TRUE(geom.valid());
  uint32_t max = 5 * 7 * 11 * 3;
  ASSERT_EQ(max, geom.max_pixel());
  ASSERT_EQ(1, geom.pixel2D(0, 0));
  ASSERT_EQ(1, geom.pixelMP2D(0, 0, 0));
  ASSERT_EQ(1, geom.pixel3D(0, 0, 0));
  ASSERT_EQ(1, geom.pixelMP3D(0, 0, 0, 0));
  ASSERT_EQ(max, geom.pixelMP3D(4, 6, 10, 2));
  ASSERT_EQ(1, geom.valid_id(max));
  ASSERT_EQ(0, geom.valid_id(max + 1));
  ASSERT_EQ(0, geom.valid_id(-1));
}

TEST_F(ESSGeometryTest, ConstructorInvalidParms) {
  ESSGeometry sp_2d(5,7, 0, 0);
  ASSERT_FALSE(sp_2d.valid());
  ASSERT_EQ(0, sp_2d.max_pixel());
  ASSERT_EQ(0, sp_2d.pixel2D(0, 0));
  ASSERT_EQ(0, sp_2d.pixelMP2D(0, 0, 0));
  ASSERT_EQ(0, sp_2d.pixel3D(0, 0, 0));
  ASSERT_EQ(0, sp_2d.pixelMP3D(0, 0, 0, 0));
  ASSERT_EQ(0, sp_2d.valid_id(0));
  ASSERT_EQ(0, sp_2d.valid_id(1));
}

TEST_F(ESSGeometryTest, SettersGetters) {
  ESSGeometry geom;
  ASSERT_EQ(0, geom.nx());
  ASSERT_EQ(0, geom.ny());
  ASSERT_EQ(0, geom.nz());
  ASSERT_EQ(0, geom.np());

  geom.nx(2);
  ASSERT_EQ(2, geom.nx());

  geom.ny(2);
  ASSERT_EQ(2, geom.ny());

  geom.nz(2);
  ASSERT_EQ(2, geom.nz());

  geom.np(2);
  ASSERT_EQ(2, geom.np());

  ASSERT_TRUE(geom.valid());
  uint32_t max = 2 * 2 * 2 * 2;
  ASSERT_EQ(max, geom.max_pixel());
}

TEST_F(ESSGeometryTest, SinglePanel2D) {
  uint32_t nx = 997;
  uint32_t ny = 991;
  ESSGeometry geom(nx, ny, 1, 1);
  ASSERT_EQ(nx*ny, geom.max_pixel());
  for (uint32_t y = 0; y < ny; y++)
    for (uint32_t x = 0; x < nx; x++) {
      ASSERT_EQ(y * nx + x + 1, geom.pixel2D(x, y));
    }
}

TEST_F(ESSGeometryTest, MultiPanel2D) {
  uint32_t nx = 997;
  uint32_t ny = 991;
  uint32_t np = 3;
  ESSGeometry geom(nx, ny, 1, np);
  ASSERT_EQ(nx*ny*np, geom.max_pixel());
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
  ASSERT_EQ(nx*ny*nz, geom.max_pixel());
  for (uint32_t z = 0; z < nz; z++)
    for (uint32_t y = 0; y < ny; y++)
      for (uint32_t x = 0; x < nx; x++) {
        ASSERT_EQ(z * nx * ny + y * nx + x + 1, geom.pixel3D(x, y, z));
      }
}

TEST_F(ESSGeometryTest, MultiPanel3D) {
  uint32_t nx = 397;
  uint32_t ny = 491;
  uint32_t nz = 16;
  uint32_t np = 3;
  ESSGeometry geom(nx, ny, nz, np);
  ASSERT_EQ(nx*ny*nz*np, geom.max_pixel());
  for (uint32_t p = 0; p < np; p++)
    for (uint32_t z = 0; z < nz; z++)
      for (uint32_t y = 0; y < ny; y++)
        for (uint32_t x = 0; x < nx; x++) {
          ASSERT_EQ(p * nx * ny * nz + z * nx * ny + y * nx + x + 1, geom.pixelMP3D(x, y, z, p));
        }
}


TEST_F(ESSGeometryTest, GetCoordinates2x2x2x2) {
  ESSGeometry geom(2, 2, 2, 2);
  ASSERT_EQ(geom.x(1), 0);
  ASSERT_EQ(geom.y(1), 0);
  ASSERT_EQ(geom.z(1), 0);
  ASSERT_EQ(geom.p(1), 0);

  ASSERT_EQ(geom.x(2), 1);
  ASSERT_EQ(geom.y(2), 0);
  ASSERT_EQ(geom.z(2), 0);
  ASSERT_EQ(geom.p(2), 0);

  ASSERT_EQ(geom.x(3), 0);
  ASSERT_EQ(geom.y(3), 1);
  ASSERT_EQ(geom.z(3), 0);
  ASSERT_EQ(geom.p(3), 0);

  ASSERT_EQ(geom.x(4), 1);
  ASSERT_EQ(geom.y(4), 1);
  ASSERT_EQ(geom.z(4), 0);
  ASSERT_EQ(geom.p(4), 0);

  ASSERT_EQ(geom.x(5), 0);
  ASSERT_EQ(geom.y(5), 0);
  ASSERT_EQ(geom.z(5), 1);
  ASSERT_EQ(geom.p(5), 0);

  ASSERT_EQ(geom.x(6), 1);
  ASSERT_EQ(geom.y(6), 0);
  ASSERT_EQ(geom.z(6), 1);
  ASSERT_EQ(geom.p(6), 0);

  ASSERT_EQ(geom.x(7), 0);
  ASSERT_EQ(geom.y(7), 1);
  ASSERT_EQ(geom.z(7), 1);
  ASSERT_EQ(geom.p(7), 0);

  ASSERT_EQ(geom.x(8), 1);
  ASSERT_EQ(geom.y(8), 1);
  ASSERT_EQ(geom.z(8), 1);
  ASSERT_EQ(geom.p(8), 0);

  ASSERT_EQ(geom.x(9), 0);
  ASSERT_EQ(geom.y(9), 0);
  ASSERT_EQ(geom.z(9), 0);
  ASSERT_EQ(geom.p(9), 1);

  ASSERT_EQ(geom.x(10), 1);
  ASSERT_EQ(geom.y(10), 0);
  ASSERT_EQ(geom.z(10), 0);
  ASSERT_EQ(geom.p(10), 1);

  ASSERT_EQ(geom.x(11), 0);
  ASSERT_EQ(geom.y(11), 1);
  ASSERT_EQ(geom.z(11), 0);
  ASSERT_EQ(geom.p(11), 1);

  ASSERT_EQ(geom.x(12), 1);
  ASSERT_EQ(geom.y(12), 1);
  ASSERT_EQ(geom.z(12), 0);
  ASSERT_EQ(geom.p(12), 1);

  ASSERT_EQ(geom.x(13), 0);
  ASSERT_EQ(geom.y(13), 0);
  ASSERT_EQ(geom.z(13), 1);
  ASSERT_EQ(geom.p(13), 1);

  ASSERT_EQ(geom.x(14), 1);
  ASSERT_EQ(geom.y(14), 0);
  ASSERT_EQ(geom.z(14), 1);
  ASSERT_EQ(geom.p(14), 1);

  ASSERT_EQ(geom.x(15), 0);
  ASSERT_EQ(geom.y(15), 1);
  ASSERT_EQ(geom.z(15), 1);
  ASSERT_EQ(geom.p(15), 1);

  ASSERT_EQ(geom.x(16), 1);
  ASSERT_EQ(geom.y(16), 1);
  ASSERT_EQ(geom.z(16), 1);
  ASSERT_EQ(geom.p(16), 1);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
