/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/geometry/DigitalGeometry.h>
#include <test/TestBase.h>

using namespace Multigrid;

class DigitalGeometryTest : public TestBase {
protected:
  DetectorGeometry geo;
  ModuleGeometry bus, bus2;

  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DigitalGeometryTest, Nothing) {
  EXPECT_EQ(geo.max_x(), 0);
  EXPECT_EQ(geo.max_y(), 0);
  EXPECT_EQ(geo.max_z(), 0);

  for (int i = 0; i < 300; i++) {
    EXPECT_ANY_THROW(geo.x_from_wire(0, i));
    EXPECT_ANY_THROW(geo.z_from_wire(0, i));
    EXPECT_ANY_THROW(geo.y_from_grid(0, i));
  }
}

TEST_F(DigitalGeometryTest, OneBus) {
  geo.add_bus(bus);

  EXPECT_EQ(geo.max_x(), 4);
  EXPECT_EQ(geo.max_y(), 40);
  EXPECT_EQ(geo.max_z(), 20);

  for (int i = 0; i < bus.num_grids(); i++) {
    EXPECT_EQ(geo.y_from_grid(0, i), bus.y_from_grid(i));
    EXPECT_ANY_THROW(geo.y_from_grid(1, i));
  }

  for (int i = 0; i < bus.num_wires(); i++) {
    EXPECT_EQ(geo.x_from_wire(0, i), bus.x_from_wire(i));
    EXPECT_ANY_THROW(geo.x_from_wire(1, i));

    EXPECT_EQ(geo.z_from_wire(0, i), bus.z_from_wire(i));
    EXPECT_ANY_THROW(geo.z_from_wire(1, i));
  }
}

TEST_F(DigitalGeometryTest, TwoBuses) {

  // confirms that they are stacked in x only

  geo.add_bus(bus);

  bus2.x_offset = bus.x_range();
  geo.add_bus(bus2);

  EXPECT_EQ(geo.max_x(), 8);
  EXPECT_EQ(geo.max_y(), 40);
  EXPECT_EQ(geo.max_z(), 20);

  for (int i = 0; i < bus.num_grids(); i++) {
    EXPECT_EQ(geo.y_from_grid(0, i), bus.y_from_grid(i));
    EXPECT_EQ(geo.y_from_grid(1, i), bus.y_from_grid(i));
    EXPECT_ANY_THROW(geo.y_from_grid(2, i));
  }

  for (int i = 0; i < bus.num_wires(); i++) {
    EXPECT_EQ(geo.x_from_wire(0, i), bus.x_from_wire(i));
    EXPECT_EQ(geo.x_from_wire(1, i), bus.x_from_wire(i) + bus.x_range());
    EXPECT_ANY_THROW(geo.x_from_wire(2, i));

    EXPECT_EQ(geo.z_from_wire(0, i), bus.z_from_wire(i));
    EXPECT_EQ(geo.z_from_wire(1, i), bus.z_from_wire(i));
    EXPECT_ANY_THROW(geo.z_from_wire(2, i));
  }
}

TEST_F(DigitalGeometryTest, PrintsSelf) {
  geo.add_bus(bus);
  bus2.x_offset = bus.x_range();
  geo.add_bus(bus2);
  EXPECT_FALSE(geo.debug().empty());
  MESSAGE() << "\n" << geo.debug() << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
