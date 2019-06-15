/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/geometry/ModuleGeometry.h>
#include <test/TestBase.h>

using namespace Multigrid;

class ModuleGeometryTest : public TestBase {
protected:
  ModuleLogicalGeometry geo;
  void SetUp() override {}
  void TearDown()  override {}
};


TEST_F(ModuleGeometryTest, GetX) {
  EXPECT_EQ(geo.max_wire(), 80);
  EXPECT_EQ(geo.max_z(), 20);
  EXPECT_EQ(geo.max_x(), 4);

  EXPECT_EQ(geo.x_from_wire(0), 0);
  EXPECT_EQ(geo.x_from_wire(19), 0);
  EXPECT_EQ(geo.x_from_wire(20), 1);
  EXPECT_EQ(geo.x_from_wire(39), 1);
  EXPECT_EQ(geo.x_from_wire(60), 3);
  EXPECT_EQ(geo.x_from_wire(79), 3);
}

TEST_F(ModuleGeometryTest, FlippedX) {
  EXPECT_EQ(geo.max_wire(), 80);
  EXPECT_EQ(geo.max_z(), 20);
  EXPECT_EQ(geo.max_x(), 4);
  geo.flipped_x(true);

  EXPECT_EQ(geo.x_from_wire(0), 3);
  EXPECT_EQ(geo.x_from_wire(19), 3);
  EXPECT_EQ(geo.x_from_wire(20), 2);
  EXPECT_EQ(geo.x_from_wire(39), 2);
  EXPECT_EQ(geo.x_from_wire(60), 0);
  EXPECT_EQ(geo.x_from_wire(79), 0);
}

TEST_F(ModuleGeometryTest, GetY) {
  EXPECT_EQ(geo.max_wire(), 80);
  EXPECT_EQ(geo.max_grid(), 40);
  EXPECT_EQ(geo.max_y(), 40);

  EXPECT_EQ(geo.y_from_grid(0), 0);
  EXPECT_EQ(geo.y_from_grid(1), 1);
  EXPECT_EQ(geo.y_from_grid(38), 38);
  EXPECT_EQ(geo.y_from_grid(39), 39);
}

TEST_F(ModuleGeometryTest, GetZ) {
  EXPECT_EQ(geo.max_wire(), 80);
  geo.max_z(10);
  EXPECT_EQ(geo.max_x(), 8);

  EXPECT_EQ(geo.z_from_wire(0), 0);
  EXPECT_EQ(geo.z_from_wire(2), 2);
  EXPECT_EQ(geo.z_from_wire(19), 9);
  EXPECT_EQ(geo.z_from_wire(20), 0);
  EXPECT_EQ(geo.z_from_wire(39), 9);
}

TEST_F(ModuleGeometryTest, FlippedZ) {
  EXPECT_EQ(geo.max_wire(), 80);
  EXPECT_EQ(geo.max_z(), 20);
  geo.flipped_z(true);

  EXPECT_EQ(geo.z_from_wire(0), 19);
  EXPECT_EQ(geo.z_from_wire(2), 17);
  EXPECT_EQ(geo.z_from_wire(19), 0);
  EXPECT_EQ(geo.z_from_wire(20), 19);
  EXPECT_EQ(geo.z_from_wire(39), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
