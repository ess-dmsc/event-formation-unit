// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <freia/geometry/Geometry.h>

using namespace Freia;

class GeometryTest : public TestBase {
protected:
  Geometry Geom;
  uint16_t VMM0{0};
  uint16_t VMM1{1};

  void SetUp() override {}
  void TearDown() override {}
};

// clang-format off

// Should match the ICD
TEST_F(GeometryTest, DefaultFreia) {
  ASSERT_TRUE(Geom.isXCoord(VMM1));
  ASSERT_TRUE(Geom.isYCoord(VMM0));
}

// Freia is already default so we need to first swap to AMOR
TEST_F(GeometryTest, SelectFreia) {
  ASSERT_TRUE(Geom.setGeometry("AMOR"));
  ASSERT_TRUE(Geom.setGeometry("Freia"));

  ASSERT_TRUE(Geom.isXCoord(VMM1));
  ASSERT_TRUE(Geom.isYCoord(VMM0));
}

// Checking that correct 2D geometry is loaded
TEST_F(GeometryTest, FreiaPixels) {
  // Test using the Freia ICD dimensions
  Geom.setGeometry("AMOR");
  Geom.setGeometry("Freia");
  const uint16_t nx = 64;
  const uint16_t ny = 1024;

  ASSERT_EQ(1, Geom.pixel2D(     0,      0));
  ASSERT_TRUE( Geom.pixel2D(nx - 1,      0) > 0);
  ASSERT_EQ(0, Geom.pixel2D(    nx,      0));
  ASSERT_TRUE( Geom.pixel2D(     0, ny - 1) > 0);
  ASSERT_EQ(0, Geom.pixel2D(     0,     ny));

  // Test that all four corners of the pixel geometry are mapped to the expected pixel id
  std::vector<std::tuple<uint16_t, uint16_t, uint16_t, uint32_t>> testData = {
  // #     x       y       Pixel ID
    {1,      0,      0,               1},
    {2, nx - 1,      0,              nx},
    {3,      0, ny - 1, nx*(ny - 1) + 1},
    {4, nx - 1, ny - 1,           nx*ny},
  };
  for (const auto &[corner, x, y, id]: testData) {
    ASSERT_EQ(id, Geom.pixel2D(x, y)) << "Pixel ID test failed for corner #" << std::to_string(corner);
  }
}

// x- and y- vmms are swapped compared with Freia
TEST_F(GeometryTest, SelectAMOR) {
  ASSERT_TRUE(Geom.setGeometry("AMOR"));
  ASSERT_TRUE(Geom.isXCoord(VMM0));
  ASSERT_TRUE(Geom.isYCoord(VMM1));
}

// Checking that correct 2D geometry is loaded
TEST_F(GeometryTest, AMORPixels) {
  // Test using the AMOR ICD dimensions
  Geom.setGeometry("AMOR");
  const uint16_t nx = 64;
  const uint16_t ny = 448;

  ASSERT_EQ(1, Geom.pixel2D(     0,      0));
  ASSERT_TRUE( Geom.pixel2D(nx - 1,      0) > 0);
  ASSERT_EQ(0, Geom.pixel2D(    nx,      0));
  ASSERT_TRUE( Geom.pixel2D(     0, ny - 1) > 0);
  ASSERT_EQ(0, Geom.pixel2D(     0,     ny));

  // Test that all four corners of the pixel geometry are mapped to the expected pixel id
  std::vector<std::tuple<uint16_t, uint16_t, uint16_t, uint32_t>> testData = {
  // #     x       y       Pixel ID
    {1,      0,      0,               1},
    {2, nx - 1,      0,              nx},
    {3,      0, ny - 1, nx*(ny - 1) + 1},
    {4, nx - 1, ny - 1,           nx*ny},
  };
  for (const auto &[corner, x, y, id]: testData) {
    ASSERT_EQ(id, Geom.pixel2D(x, y)) << "Pixel ID test failed for corner #" << std::to_string(corner);
  }
}

TEST_F(GeometryTest, SelectEstia) {
  ASSERT_TRUE(Geom.setGeometry("Estia"));
  ASSERT_TRUE(Geom.isXCoord(VMM1));
  ASSERT_TRUE(Geom.isYCoord(VMM0));
}

// Checking that correct 2D geometry is loaded
TEST_F(GeometryTest, EstiaPixels) {
  // Test using the ESTIA ICD dimensions
  Geom.setGeometry("Estia");
  const uint16_t nx = 1536;
  const uint16_t ny = 128;

  ASSERT_EQ(1, Geom.pixel2D(     0,      0));
  ASSERT_TRUE( Geom.pixel2D(nx - 1,      0) > 0);
  ASSERT_EQ(0, Geom.pixel2D(    nx,      0));
  ASSERT_TRUE( Geom.pixel2D(     0, ny - 1) > 0);
  ASSERT_EQ(0, Geom.pixel2D(     0,     ny));

  // Check that https://jira.ess.eu/browse/ECDC-4545 is fixed by testing that
  // all four corners of the pixel geometry are mapped to the expected pixel id
  std::vector<std::tuple<uint16_t, uint16_t, uint16_t, uint32_t>> testData = {
  // #     x       y       Pixel ID
    {1,      0,      0,               1},
    {2, nx - 1,      0,              nx},
    {3,      0, ny - 1, nx*(ny - 1) + 1},
    {4, nx - 1, ny - 1,           nx*ny},
  };
  for (const auto &[corner, x, y, id]: testData) {
    ASSERT_EQ(id, Geom.pixel2D(x, y)) << "Pixel ID test failed for corner #" << std::to_string(corner);
  }
}

TEST_F(GeometryTest, SelectInvalid) {
  ASSERT_FALSE(Geom.setGeometry(""));
  ASSERT_TRUE(Geom.isXCoord(VMM1));
  ASSERT_TRUE(Geom.isYCoord(VMM0));

  ASSERT_FALSE(Geom.setGeometry("InvalidInstrument"));
  ASSERT_TRUE(Geom.isXCoord(VMM1));
  ASSERT_TRUE(Geom.isYCoord(VMM0));
}

// clang-format on

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
