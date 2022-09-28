// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <dream/geometry/Cuboid.h>
#include <dream/readout/DataParser.h>

using namespace Dream;

class CDTGeometryTest : public TestBase {
protected:
  DataParser::DreamReadout Readout{0, 0, 0, 0, 0, 0, 0, 0, 0};
  Config::ModuleParms Parms{false, Config::ModuleType::HR, {0}, {0}};
  Cuboid geometry;
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(CDTGeometryTest, Rotate) {
  int x = 1;
  int y = 12;
  geometry.rotateXY(x, y, 0);
  ASSERT_EQ(x,  1);
  ASSERT_EQ(y, 12);
  geometry.rotateXY(x, y, 1);
  ASSERT_NE(x,  1);
  ASSERT_NE(y, 12);
  geometry.rotateXY(x, y, 1);
  geometry.rotateXY(x, y, 1);
  geometry.rotateXY(x, y, 1);
  ASSERT_EQ(x,  1);
  ASSERT_EQ(y, 12);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
