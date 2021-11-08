// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <freia/geometry/Geometry.h>
#include <common/testutils/TestBase.h>

using namespace Freia;

class GeometryTest : public TestBase {
protected:
  Geometry Geom;
  uint16_t VMM0{0};
  uint16_t VMM1{1};

  void SetUp() override {}
  void TearDown() override {}
};


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

// x- and y- vmms are swapped compared with Freia
TEST_F(GeometryTest, SelectAMOR) {
  ASSERT_TRUE(Geom.setGeometry("AMOR"));
  ASSERT_TRUE(Geom.isXCoord(VMM0));
  ASSERT_TRUE(Geom.isYCoord(VMM1));
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
