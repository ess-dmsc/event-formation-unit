// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <dream/geometry/Cuboid.h>
#include <dream/readout/DataParser.h>

using namespace Dream;

class CuboidGeometryTest : public TestBase {
protected:
  DataParser::DreamReadout Readout{0, 0, 0, 0, 0, 0, 0, 0, 0};
  Config::ModuleParms Parms{false, Config::ModuleType::HR, {0}, {0}};
  Cuboid geometry;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(CuboidGeometryTest, Rotate0) {
  int x = 1;
  int y = 12;
  geometry.rotateXY(x, y, 0);
  ASSERT_EQ(x, 1);
  ASSERT_EQ(y, 12);
}

TEST_F(CuboidGeometryTest, Rotate1) {
  int x = 1;
  int y = 12;
  geometry.rotateXY(x, y, 1);
  ASSERT_NE(x, 1);
  ASSERT_NE(y, 12);
  geometry.rotateXY(x, y, 1);
  geometry.rotateXY(x, y, 1);
  geometry.rotateXY(x, y, 1);
  ASSERT_EQ(x, 1);
  ASSERT_EQ(y, 12);
}

TEST_F(CuboidGeometryTest, Rotate2) {
  int x = 1;
  int y = 12;
  geometry.rotateXY(x, y, 2);
  ASSERT_NE(x, 1);
  ASSERT_NE(y, 12);
  geometry.rotateXY(x, y, 2);
  ASSERT_EQ(x, 1);
  ASSERT_EQ(y, 12);
}

TEST_F(CuboidGeometryTest, Rotate3) {
  int x = 1;
  int y = 12;
  geometry.rotateXY(x, y, 3);
  ASSERT_NE(x, 1);
  ASSERT_NE(y, 12);
  geometry.rotateXY(x, y, 1);
  ASSERT_EQ(x, 1);
  ASSERT_EQ(y, 12);
}

TEST_F(CuboidGeometryTest, OffsetRange) {
  Parms.P1.Index = 32;
  ASSERT_NE(geometry.getPixelId(Parms, Readout), -1);
  Parms.P1.Index = 33;
  ASSERT_EQ(geometry.getPixelId(Parms, Readout), -1);

  Parms.Type = Config::ModuleType::SANS;
  Parms.P1.Index = 35;
  ASSERT_NE(geometry.getPixelId(Parms, Readout), -1);
  Parms.P1.Index = 36;
  ASSERT_EQ(geometry.getPixelId(Parms, Readout), -1);

  Parms.Type = Config::ModuleType::FwEndCap;
  Parms.P1.Index = 0;
  ASSERT_EQ(geometry.getPixelId(Parms, Readout), -1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
