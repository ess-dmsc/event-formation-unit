// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <freia/geometry/FreiaGeometry.h>
#include <freia/geometry/AmorGeometry.h>
#include <freia/geometry/EstiaGeometry.h>
#include <freia/geometry/TBLMBGeometry.h>
#include <freia/geometry/Config.h>
#include <common/Statistics.h>

using namespace Freia;

class GeometryTest : public TestBase {
protected:
  Statistics Stats;
  Config Conf; // default InstrumentGeometry Freia
  FreiaGeometry FreiaGeom{Stats, Conf};
  AmorGeometry AmorGeom{Stats, Conf};
  EstiaGeometry EstiaGeom{Stats, Conf};
  TBLMBGeometry TBLMBGeom{Stats, Conf};
  uint16_t VMM0{0};
  uint16_t VMM1{1};
  void SetUp() override {}
  void TearDown() override {}
};

// clang-format off

// Should match the ICD
TEST_F(GeometryTest, FreiaMapping) {
  ASSERT_TRUE(FreiaGeom.isXCoord(VMM1));
  ASSERT_TRUE(FreiaGeom.isYCoord(VMM0));
}

// Freia is already default so we need to first swap to AMOR
TEST_F(GeometryTest, AMORMappings) {
  ASSERT_TRUE(AmorGeom.isXCoord(VMM0));
  ASSERT_TRUE(AmorGeom.isYCoord(VMM1));
}

// Checking that correct 2D geometry is loaded
TEST_F(GeometryTest, FreiaPixels) {
  const uint16_t nx = 64;
  const uint16_t ny = 1024;
  ASSERT_EQ(1, FreiaGeom.pixel2D(0, 0));
  ASSERT_TRUE(FreiaGeom.pixel2D(nx - 1, 0) > 0);
  ASSERT_EQ(0, FreiaGeom.pixel2D(nx, 0));
  ASSERT_TRUE(FreiaGeom.pixel2D(0, ny - 1) > 0);
  ASSERT_EQ(0, FreiaGeom.pixel2D(0, ny));

  // Test that all four corners of the pixel geometry are mapped to the expected pixel id
  std::vector<std::tuple<uint16_t, uint16_t, uint16_t, uint32_t>> testData = {
  // #     x       y       Pixel ID
    {1,      0,      0,               1},
    {2, nx - 1,      0,              nx},
    {3,      0, ny - 1, nx*(ny - 1) + 1},
    {4, nx - 1, ny - 1,           nx*ny},
  };
  for (const auto &[corner, x, y, id]: testData) {
    ASSERT_EQ(id, FreiaGeom.pixel2D(x, y)) << "Pixel ID test failed for corner #" << std::to_string(corner);
  }
}

// x- and y- vmms are swapped compared with Freia
TEST_F(GeometryTest, AMORPixels) {
  const uint16_t nx = 64;
  const uint16_t ny = 448;
  ASSERT_EQ(1, AmorGeom.pixel2D(0, 0));
  ASSERT_TRUE(AmorGeom.pixel2D(nx - 1, 0) > 0);
  ASSERT_EQ(0, AmorGeom.pixel2D(nx, 0));
  ASSERT_TRUE(AmorGeom.pixel2D(0, ny - 1) > 0);
  ASSERT_EQ(0, AmorGeom.pixel2D(0, ny));

// Checking that correct 2D geometry is loaded
// clang-format off
  std::vector<std::tuple<uint16_t, uint16_t, uint16_t, uint32_t>> testData = {
    {1,     0,        0,        1},
    {2,     nx - 1,   0,        nx},
    {3,     0,        ny - 1,   nx*(ny - 1) + 1},
    {4,     nx - 1,   ny - 1,   nx*ny},
  };
  // clang-format on
  for (const auto &[corner, x, y, id]: testData) { ASSERT_EQ(id, AmorGeom.pixel2D(x, y)); }
}

TEST_F(GeometryTest, EstiaMapping) {
  ASSERT_TRUE(EstiaGeom.isXCoord(VMM1));
  ASSERT_TRUE(EstiaGeom.isYCoord(VMM0));
}

// Checking that correct 2D geometry is loaded
TEST_F(GeometryTest, EstiaPixels) {
  const uint16_t nx = 1536; const uint16_t ny = 128;
  ASSERT_EQ(1, EstiaGeom.pixel2D(0, 0));
  ASSERT_TRUE(EstiaGeom.pixel2D(nx - 1, 0) > 0);
  ASSERT_EQ(0, EstiaGeom.pixel2D(nx, 0));
  ASSERT_TRUE(EstiaGeom.pixel2D(0, ny - 1) > 0);
  ASSERT_EQ(0, EstiaGeom.pixel2D(0, ny));
  std::vector<std::tuple<uint16_t, uint16_t, uint16_t, uint32_t>> testData = {
    {1,0,0,1},{2,nx-1,0,nx},{3,0,ny-1,nx*(ny-1)+1},{4,nx-1,ny-1,nx*ny}, };
  for (const auto &[corner,x,y,id]: testData) { ASSERT_EQ(id, EstiaGeom.pixel2D(x,y)); }
}

TEST_F(GeometryTest, TBLMBMapping) {
  ASSERT_TRUE(TBLMBGeom.isXCoord(VMM0));
  ASSERT_TRUE(TBLMBGeom.isYCoord(VMM1));
}

// clang-format on

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
