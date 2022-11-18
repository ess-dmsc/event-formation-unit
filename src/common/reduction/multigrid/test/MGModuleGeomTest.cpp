// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief unit test for multigrid ModuleGeometry class
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>

#include <common/reduction/multigrid/ModuleGeometry.h>

class MGModuleGeomTest : public TestBase {
protected:
  Multigrid::ModuleGeometry MgGeom;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MGModuleGeomTest, Constructor) {
  ASSERT_FALSE(MgGeom.flipped_x());
  ASSERT_FALSE(MgGeom.flipped_z());
  ASSERT_EQ(MgGeom.num_grids(), 40);
  ASSERT_EQ(MgGeom.num_wires(), 80);
}

TEST_F(MGModuleGeomTest, FlippedXZ) {
  MgGeom.flipped_x(true);
  MgGeom.flipped_z(true);
  ASSERT_TRUE(MgGeom.flipped_x());
  ASSERT_TRUE(MgGeom.flipped_z());
}

TEST_F(MGModuleGeomTest, XFromWire) {
  ASSERT_EQ(MgGeom.x_from_wire(0), 0);
  MgGeom.flipped_x(true);
  ASSERT_EQ(MgGeom.x_from_wire(0), 3);
}

TEST_F(MGModuleGeomTest, ZFromWire) {
  ASSERT_EQ(MgGeom.z_from_wire(0), 0);
  MgGeom.flipped_z(true);
  ASSERT_EQ(MgGeom.z_from_wire(0), 19);
}

TEST_F(MGModuleGeomTest, NumWiresGrids) {
  MgGeom.num_grids(140);
  MgGeom.num_wires(96);
  ASSERT_EQ(MgGeom.num_grids(), 140);
  ASSERT_EQ(MgGeom.num_wires(), 96);
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
