/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/geometry/BusGeometry.h>
#include <test/TestBase.h>

using namespace Multigrid;

class BusGeometryTest : public TestBase {
protected:
  BusGeometry geo;
  Filter f;
  virtual void SetUp() {
    f.minimum = 3;
    f.maximum = 7;
    f.rescale_factor = 0.5;
  }
  virtual void TearDown() {
  }
};


TEST_F(BusGeometryTest, DefaultConstructedValues) {
  EXPECT_EQ(geo.max_channel(), 120);
  EXPECT_EQ(geo.max_wire(), 80);
  EXPECT_EQ(geo.max_z(), 20);
  EXPECT_FALSE(geo.swap_wires());
  EXPECT_FALSE(geo.swap_grids());
  EXPECT_FALSE(geo.flipped_x());
  EXPECT_FALSE(geo.flipped_z());
}

TEST_F(BusGeometryTest, WireLimits) {
  geo.max_wire(40);
  EXPECT_EQ(geo.max_wire(), 40);
  EXPECT_TRUE(geo.isWire(0));
  EXPECT_TRUE(geo.isWire(2));
  EXPECT_TRUE(geo.isWire(39));
  EXPECT_FALSE(geo.isWire(40));
  EXPECT_FALSE(geo.isWire(7000));
}

TEST_F(BusGeometryTest, GridLimits) {
  geo.max_channel(100);
  EXPECT_EQ(geo.max_channel(), 100);
  EXPECT_EQ(geo.max_grid(), 20);
  EXPECT_FALSE(geo.isGrid(0));
  EXPECT_FALSE(geo.isGrid(79));
  EXPECT_TRUE(geo.isGrid(80));
  EXPECT_TRUE(geo.isGrid(99));
  EXPECT_FALSE(geo.isGrid(100));
  EXPECT_FALSE(geo.isWire(100));
}

TEST_F(BusGeometryTest, GetWire) {
  geo.max_wire(40);
  EXPECT_EQ(geo.max_wire(), 40);
  EXPECT_EQ(geo.wire(0), 0);
  EXPECT_EQ(geo.wire(2), 2);
  EXPECT_EQ(geo.wire(39), 39);

  // undefined for out of bounds
}

TEST_F(BusGeometryTest, SwapWires) {
  geo.swap_wires(true);
  EXPECT_TRUE(geo.swap_wires());
  EXPECT_EQ(geo.max_wire(), 80);

  EXPECT_EQ(geo.wire(0), 1);
  EXPECT_EQ(geo.wire(1), 0);
  EXPECT_EQ(geo.wire(39), 38);
  EXPECT_EQ(geo.wire(38), 39);

  // undefined for out of bounds
}

TEST_F(BusGeometryTest, GetGrid) {
  geo.max_channel(100);
  EXPECT_EQ(geo.grid(80), 0);
  EXPECT_EQ(geo.grid(99), 19);

  // undefined for out of bounds
}

TEST_F(BusGeometryTest, SwapGrids) {
  geo.swap_grids(true);
  EXPECT_TRUE(geo.swap_grids());
  EXPECT_EQ(geo.max_grid(), 40);
  EXPECT_EQ(geo.max_channel(), 120);

  EXPECT_EQ(geo.grid(80), 1);
  EXPECT_EQ(geo.grid(81), 0);
  EXPECT_EQ(geo.grid(98), 19);
  EXPECT_EQ(geo.grid(99), 18);

  // undefined for out of bounds
}

TEST_F(BusGeometryTest, GetX) {
  EXPECT_FALSE(geo.swap_wires());
  EXPECT_EQ(geo.max_wire(), 80);
  EXPECT_EQ(geo.max_z(), 20);
  EXPECT_EQ(geo.max_x(), 4);

  EXPECT_EQ(geo.x_from_wire(0), 0);
  EXPECT_EQ(geo.x_from_wire(19), 0);
  EXPECT_EQ(geo.x_from_wire(20), 1);
  EXPECT_EQ(geo.x_from_wire(39), 1);
  EXPECT_EQ(geo.x_from_wire(60), 3);
  EXPECT_EQ(geo.x_from_wire(79), 3);

  EXPECT_EQ(geo.x(0), 0);
  EXPECT_EQ(geo.x(19), 0);
  EXPECT_EQ(geo.x(20), 1);
  EXPECT_EQ(geo.x(39), 1);
  EXPECT_EQ(geo.x(60), 3);
  EXPECT_EQ(geo.x(79), 3);
}

TEST_F(BusGeometryTest, FlippedX) {
  EXPECT_FALSE(geo.swap_wires());
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

  EXPECT_EQ(geo.x(0), 3);
  EXPECT_EQ(geo.x(19), 3);
  EXPECT_EQ(geo.x(20), 2);
  EXPECT_EQ(geo.x(39), 2);
  EXPECT_EQ(geo.x(60), 0);
  EXPECT_EQ(geo.x(79), 0);
}

TEST_F(BusGeometryTest, GetY) {
  EXPECT_FALSE(geo.swap_grids());
  EXPECT_EQ(geo.max_channel(), 120);
  EXPECT_EQ(geo.max_wire(), 80);
  EXPECT_EQ(geo.max_grid(), 40);
  EXPECT_EQ(geo.max_y(), 40);

  EXPECT_EQ(geo.y_from_grid(0), 0);
  EXPECT_EQ(geo.y_from_grid(1), 1);
  EXPECT_EQ(geo.y_from_grid(38), 38);
  EXPECT_EQ(geo.y_from_grid(39), 39);

  EXPECT_EQ(geo.y(80), 0);
  EXPECT_EQ(geo.y(81), 1);
  EXPECT_EQ(geo.y(118), 38);
  EXPECT_EQ(geo.y(119), 39);
}

TEST_F(BusGeometryTest, GetZ) {
  EXPECT_FALSE(geo.swap_wires());
  EXPECT_EQ(geo.max_wire(), 80);
  geo.max_z(10);
  EXPECT_EQ(geo.max_x(), 8);

  EXPECT_EQ(geo.z_from_wire(0), 0);
  EXPECT_EQ(geo.z_from_wire(2), 2);
  EXPECT_EQ(geo.z_from_wire(19), 9);
  EXPECT_EQ(geo.z_from_wire(20), 0);
  EXPECT_EQ(geo.z_from_wire(39), 9);

  EXPECT_EQ(geo.z(0), 0);
  EXPECT_EQ(geo.z(2), 2);
  EXPECT_EQ(geo.z(19), 9);
  EXPECT_EQ(geo.z(20), 0);
  EXPECT_EQ(geo.z(39), 9);
}

TEST_F(BusGeometryTest, FlippedZ) {
  EXPECT_FALSE(geo.swap_wires());
  EXPECT_EQ(geo.max_wire(), 80);
  EXPECT_EQ(geo.max_z(), 20);
  geo.flipped_z(true);

  EXPECT_EQ(geo.z_from_wire(0), 19);
  EXPECT_EQ(geo.z_from_wire(2), 17);
  EXPECT_EQ(geo.z_from_wire(19), 0);
  EXPECT_EQ(geo.z_from_wire(20), 19);
  EXPECT_EQ(geo.z_from_wire(39), 0);

  EXPECT_EQ(geo.z(0), 19);
  EXPECT_EQ(geo.z(2), 17);
  EXPECT_EQ(geo.z(19), 0);
  EXPECT_EQ(geo.z(20), 19);
  EXPECT_EQ(geo.z(39), 0);
}

TEST_F(BusGeometryTest, OneWireFilter) {
  geo.override_wire_filter(5, f);

  EXPECT_EQ(geo.rescale_wire(4, 2), 2);
  EXPECT_TRUE(geo.valid_wire(4,10));

  EXPECT_EQ(geo.rescale_wire(5, 2), 1);
  EXPECT_FALSE(geo.valid_wire(5,10));

  EXPECT_EQ(geo.rescale_wire(6, 2), 2);
  EXPECT_TRUE(geo.valid_wire(6,10));
}

TEST_F(BusGeometryTest, BlanketWireFilter) {
  geo.set_wire_filters(f);

  EXPECT_EQ(geo.rescale_wire(1, 2), 1);
  EXPECT_FALSE(geo.valid_wire(1,10));

  EXPECT_EQ(geo.rescale_wire(5, 2), 1);
  EXPECT_FALSE(geo.valid_wire(5,10));

  EXPECT_EQ(geo.rescale_wire(70, 2), 1);
  EXPECT_FALSE(geo.valid_wire(70,10));
}

TEST_F(BusGeometryTest, OneGridFilter) {
  geo.override_grid_filter(5, f);

  EXPECT_EQ(geo.rescale_grid(4, 2), 2);
  EXPECT_TRUE(geo.valid_grid(4,10));

  EXPECT_EQ(geo.rescale_grid(5, 2), 1);
  EXPECT_FALSE(geo.valid_grid(5,10));

  EXPECT_EQ(geo.rescale_grid(6, 2), 2);
  EXPECT_TRUE(geo.valid_grid(6,10));
}

TEST_F(BusGeometryTest, BlanketGridFilter) {
  geo.set_grid_filters(f);

  EXPECT_EQ(geo.rescale_grid(1, 2), 1);
  EXPECT_FALSE(geo.valid_grid(1,10));

  EXPECT_EQ(geo.rescale_grid(5, 2), 1);
  EXPECT_FALSE(geo.valid_grid(5,10));

  EXPECT_EQ(geo.rescale_grid(30, 2), 1);
  EXPECT_FALSE(geo.valid_grid(30,10));
}

TEST_F(BusGeometryTest, PrintsSelf) {
  geo.swap_wires(true);
  geo.swap_grids(true);
  geo.flipped_x(true);
  geo.flipped_z(true);
  EXPECT_FALSE(geo.debug().empty());
}

TEST_F(BusGeometryTest, FromJsonThrows) {
  nlohmann::json j;
  j["max_channel"] = "nonsense";
  EXPECT_ANY_THROW((geo = j));
}

TEST_F(BusGeometryTest, FromJsonMinimal) {
  nlohmann::json j;
  j["max_channel"] = 100;
  j["max_wire"] = 50;
  j["max_z"] = 5;

  geo = j;

  EXPECT_EQ(geo.max_x(), 10);
  EXPECT_EQ(geo.max_y(), 50);
  EXPECT_EQ(geo.max_z(), 5);
}

TEST_F(BusGeometryTest, FromJson) {
  nlohmann::json j;
  j["max_channel"] = 100;
  j["max_wire"] = 50;
  j["max_z"] = 5;

  j["swap_wires"] = true;
  j["swap_grids"] = true;
  j["flipped_x"] = true;
  j["flipped_z"] = true;

  auto j1 = j["wire_filters"]["blanket"];
  j1["min"] = 3;
  j1["max"] = 7;
  j1["rescale"] = 0.5;

  nlohmann::json j2;
  j2["idx"] = 5;
  j2["min"] = 3;
  j2["max"] = 7;
  j2["rescale"] = 0.5;
  j["wire_filters"]["exceptions"].push_back(j2);

  geo = j;

  EXPECT_EQ(geo.max_x(), 10);
  EXPECT_EQ(geo.max_y(), 50);
  EXPECT_EQ(geo.max_z(), 5);

  // \todo test correct parsing
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
