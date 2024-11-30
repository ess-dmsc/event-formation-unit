// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <estia/geometry/Geometry.h>

using namespace Estia;

class GeometryTest : public TestBase {
protected:
  Geometry Geom;
  uint16_t VMM0{0};
  uint16_t VMM1{1};

  void SetUp() override {}
  void TearDown() override {}
};

// Should match the ICD
TEST_F(GeometryTest, DefaultEstia) {
  ASSERT_TRUE(Geom.isXCoord(VMM1));
  ASSERT_TRUE(Geom.isYCoord(VMM0));
}

TEST_F(GeometryTest, SelectInvalid) {
  ASSERT_FALSE(Geom.setGeometry(""));
  ASSERT_TRUE(Geom.isXCoord(VMM1));
  ASSERT_TRUE(Geom.isYCoord(VMM0));

  ASSERT_FALSE(Geom.setGeometry("InvalidInstrument"));
  ASSERT_TRUE(Geom.isXCoord(VMM1));
  ASSERT_TRUE(Geom.isYCoord(VMM0));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
