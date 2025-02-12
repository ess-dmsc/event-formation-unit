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

// CHecking that correct 2D geometry is loaded
TEST_F(GeometryTest, FreiaPixels) {
  Geom.setGeometry("AMOR");
  Geom.setGeometry("Freia");

  ASSERT_EQ(1, Geom.pixel2D( 0,    0));
  ASSERT_TRUE( Geom.pixel2D(63,    0) > 0);
  ASSERT_EQ(0, Geom.pixel2D(64,    0));
  ASSERT_TRUE( Geom.pixel2D( 0, 1023) > 0);
  ASSERT_EQ(0, Geom.pixel2D( 0, 1024));
}

// x- and y- vmms are swapped compared with Freia
TEST_F(GeometryTest, SelectAMOR) {
  ASSERT_TRUE(Geom.setGeometry("AMOR"));
  ASSERT_TRUE(Geom.isXCoord(VMM0));
  ASSERT_TRUE(Geom.isYCoord(VMM1));
}

// Checking that correct 2D geometry is loaded
TEST_F(GeometryTest, AMORPixels) {
  Geom.setGeometry("AMOR");
  ASSERT_EQ(1, Geom.pixel2D( 0,   0));
  ASSERT_TRUE( Geom.pixel2D(63,   0) > 0);
  ASSERT_EQ(0, Geom.pixel2D(64,   0));
  ASSERT_TRUE( Geom.pixel2D( 0, 447) > 0);
  ASSERT_EQ(0, Geom.pixel2D( 0, 448));
}


TEST_F(GeometryTest, SelectEstia) {
  ASSERT_TRUE(Geom.setGeometry("Estia"));
  ASSERT_TRUE(Geom.isXCoord(VMM1));
  ASSERT_TRUE(Geom.isYCoord(VMM0));
}

// Checking that correct 2D geometry is loaded
//GeometryInst->essgeom = new ESSGeometry(1536, 128, 1, 1);
TEST_F(GeometryTest, EstiaPixels) {
  Geom.setGeometry("Estia");
  ASSERT_EQ(1, Geom.pixel2D(   0,   0));
  ASSERT_TRUE( Geom.pixel2D(1535,   0) > 0);
  ASSERT_EQ(0, Geom.pixel2D(1536,   0));
  ASSERT_TRUE( Geom.pixel2D(   0, 127) > 0);
  ASSERT_EQ(0, Geom.pixel2D(   0, 128));
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
