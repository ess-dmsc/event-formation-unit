// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <cspec/geometry/CSPECGeometry.h>
#include <common/testutils/TestBase.h>

using namespace Cspec;

class CSPECGeometryTest : public TestBase {
protected:
  CSPECGeometry Geom;
  uint16_t VMM0{0};
  uint16_t VMM1{1};
  uint16_t VMM2{2};

  void SetUp() override {}
  void TearDown() override {}
};


// Should match the ICD
TEST_F(CSPECGeometryTest, DefaultCSPEC) {
  ASSERT_TRUE(Geom.isWire(VMM0));
  ASSERT_TRUE(Geom.isGrid(VMM1));
  ASSERT_TRUE(Geom.isGrid(VMM2));
}

TEST_F(CSPECGeometryTest, CoordinateCalculations){
  //xAndzCoord = ((4 * 12 * RindID) + (6 * FENId) + VMM & Channel specific value) * 16 + (Channel % 16)
  ASSERT_EQ(Geom.xAndzCoord(1, 0, 32, 0, false), 0);
  ASSERT_EQ(Geom.xAndzCoord(1, 1, 43, 174 ,false), 2859);
  //yCoord = VMM & Channel specific value
  ASSERT_EQ(Geom.yCoord(2, 1, 5, 0, false), 0);
  ASSERT_EQ(Geom.yCoord(1, 0, 60, 0, false), 137);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
