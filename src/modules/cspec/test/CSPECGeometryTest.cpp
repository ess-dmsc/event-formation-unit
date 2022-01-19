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

TEST_F(CSPECGeometryTest, XAndZCoordinateCalculations){
  //xAndzCoord takes HybridID, VMMID, Channel, XOffset, Rotated
  
  //Vessel 0, very first xAndzCoord position
  ASSERT_EQ(Geom.xAndzCoord(0, 0, 0, 32, 0, false), 0);
  //Vessel 14, position is mid-column
  ASSERT_EQ(Geom.xAndzCoord(0, 0, 1, 43, 168 ,false), 2763);
  //Vessel 25, rotated, column 0 and column 1
  ASSERT_EQ(Geom.xAndzCoord(0, 0, 1, 43, 300 , true), 4923);
  ASSERT_EQ(Geom.xAndzCoord(1, 0, 1, 43, 300 , true), 4827);

}

TEST_F(CSPECGeometryTest, YCoordinateCalculations){
  //yCoord = VMM & Channel specific value
  ASSERT_EQ(Geom.yCoord(1, 0, 58, 0, false, false), 0);
  ASSERT_EQ(Geom.yCoord(2, 1, 5, 0, false, false), 139);

  //short rotated vessel
  ASSERT_EQ(Geom.yCoord(1, 0, 40, 50, true, true), 10);

  //short not rotated vessel
  ASSERT_EQ(Geom.yCoord(1, 0, 40, 89, false, true), 129);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
