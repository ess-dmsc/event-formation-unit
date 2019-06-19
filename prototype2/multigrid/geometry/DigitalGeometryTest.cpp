/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/TSCTimer.h>
#include <multigrid/geometry/DigitalGeometry.h>
#include <multigrid/geometry/MGSeqMappings.h>
#include <test/TestBase.h>

using namespace Multigrid;

class DigitalGeometryTest : public TestBase {
protected:
  DigitalGeometry geo;
  BusDefinitionStruct bus;

  void SetUp() override {
    bus.channel_mappings = std::make_shared<MGSeqMappings>();
  }
  void TearDown() override {
  }
};

TEST_F(DigitalGeometryTest, Nothing) {

  EXPECT_EQ(geo.max_wire(), 0);
  EXPECT_EQ(geo.max_grid(), 0);
  EXPECT_EQ(geo.max_x(), 0);
  EXPECT_EQ(geo.max_y(), 0);
  EXPECT_EQ(geo.max_z(), 0);

  for (int i = 0; i < 120; i++) {
    EXPECT_FALSE(geo.isWire(0, i));
    EXPECT_FALSE(geo.isGrid(0, i));
  }

  for (int i = 0; i < 120; i++) {
    EXPECT_FALSE(geo.isWire(1, i));
    EXPECT_FALSE(geo.isGrid(1, i));
  }

  EXPECT_FALSE(geo.isWire(0, 128));
  EXPECT_FALSE(geo.isGrid(0, 128));
}

TEST_F(DigitalGeometryTest, OneBus) {
  geo.add_bus(bus);

  EXPECT_EQ(geo.max_wire(), 80);
  EXPECT_EQ(geo.max_grid(), 40);
  EXPECT_EQ(geo.max_x(), 4);
  EXPECT_EQ(geo.max_y(), 40);
  EXPECT_EQ(geo.max_z(), 20);

  for (int i = 0; i <= 79; i++) {
    EXPECT_TRUE(geo.isWire(0, i));
    EXPECT_FALSE(geo.isWire(1, i));
    EXPECT_FALSE(geo.isGrid(0, i));
    EXPECT_FALSE(geo.isGrid(1, i));
    EXPECT_EQ(geo.x_from_wire(geo.wire(0, i)),
              bus.logical_geometry.x_from_wire(bus.channel_mappings->wire(i)));
    EXPECT_EQ(geo.z_from_wire(geo.wire(0, i)),
              bus.logical_geometry.z_from_wire(bus.channel_mappings->wire(i)));
    EXPECT_EQ(geo.wire(0, i), bus.channel_mappings->wire(i));
  }

  for (int i = 80; i <= 119; i++) {
    EXPECT_FALSE(geo.isWire(0, i));
    EXPECT_FALSE(geo.isWire(1, i));
    EXPECT_TRUE(geo.isGrid(0, i));
    EXPECT_FALSE(geo.isGrid(1, i));
    EXPECT_EQ(geo.y_from_grid(geo.grid(0, i)),
              bus.logical_geometry.y_from_grid(bus.channel_mappings->grid(i)));
    EXPECT_EQ(geo.grid(0, i), bus.channel_mappings->grid(i));
  }

  EXPECT_FALSE(geo.isWire(0, 128));
  EXPECT_FALSE(geo.isGrid(0, 128));
}

TEST_F(DigitalGeometryTest, TwoBuses) {

  // confirms that they are stacked in x only

  geo.add_bus(bus);
  geo.add_bus(bus);

  MESSAGE() << "\n" << geo.debug() << "\n";

  EXPECT_EQ(geo.max_wire(), 160);
  EXPECT_EQ(geo.max_grid(), 80);
  EXPECT_EQ(geo.max_x(), 8);
  EXPECT_EQ(geo.max_y(), 40);
  EXPECT_EQ(geo.max_z(), 20);

  for (int i = 0; i <= 79; i++) {
    EXPECT_TRUE(geo.isWire(0, i));
    EXPECT_TRUE(geo.isWire(1, i));
    EXPECT_FALSE(geo.isWire(2, i));
    EXPECT_FALSE(geo.isGrid(0, i));
    EXPECT_FALSE(geo.isGrid(1, i));
    EXPECT_FALSE(geo.isGrid(2, i));

    EXPECT_EQ(geo.x_from_wire(geo.wire(0, i)),
              bus.logical_geometry.x_from_wire(bus.channel_mappings->wire(i)));
    EXPECT_EQ(geo.z_from_wire(geo.wire(0, i)),
              bus.logical_geometry.z_from_wire(bus.channel_mappings->wire(i)));
    EXPECT_EQ(geo.wire(0, i),
              bus.channel_mappings->wire(i));
    EXPECT_EQ(geo.wire(1, i),
              bus.channel_mappings->wire(i) + bus.logical_geometry.max_wire());

    EXPECT_EQ(geo.x_from_wire(geo.wire(1, i)),
              bus.logical_geometry.x_from_wire(bus.channel_mappings->wire(i))
              + bus.logical_geometry.max_x());
    EXPECT_EQ(geo.z_from_wire(geo.wire(1, i)),
              bus.logical_geometry.z_from_wire(bus.channel_mappings->wire(i)));
    EXPECT_EQ(geo.wire(1, i),
              bus.channel_mappings->wire(i) + bus.logical_geometry.max_wire());
    EXPECT_EQ(geo.x_from_wire(geo.wire(0, i)),
              geo.x_from_wire(geo.wire(0, i)));
    EXPECT_EQ(geo.x_from_wire(geo.wire(1, i)),
              geo.x_from_wire(geo.wire(1, i)));
    EXPECT_EQ(geo.z_from_wire(geo.wire(0, i)),
              geo.z_from_wire(geo.wire(0, i)));
    EXPECT_EQ(geo.z_from_wire(geo.wire(1, i)),
              geo.z_from_wire(geo.wire(1, i)));
  }

  for (int i = 80; i <= 119; i++) {
    EXPECT_FALSE(geo.isWire(0, i));
    EXPECT_FALSE(geo.isWire(1, i));
    EXPECT_FALSE(geo.isWire(2, i));
    EXPECT_TRUE(geo.isGrid(0, i));
    EXPECT_TRUE(geo.isGrid(1, i));
    EXPECT_FALSE(geo.isGrid(2, i));

    EXPECT_EQ(geo.grid(0, i), bus.channel_mappings->grid(i));
    EXPECT_EQ(geo.grid(1, i),
              bus.channel_mappings->grid(i) + bus.logical_geometry.max_grid());
    EXPECT_EQ(geo.y_from_grid(geo.grid(0, i)),
              bus.logical_geometry.y_from_grid(bus.channel_mappings->grid(i)));
    EXPECT_EQ(geo.y_from_grid(geo.grid(1, i)),
              bus.logical_geometry.y_from_grid(bus.channel_mappings->grid(i)));
    EXPECT_EQ(geo.y_from_grid(geo.grid(0, i)),
              geo.y_from_grid(geo.grid(0, i)));
    EXPECT_EQ(geo.y_from_grid(geo.grid(1, i)),
              geo.y_from_grid(geo.grid(1, i)));
  }

  EXPECT_FALSE(geo.isWire(0, 128));
  EXPECT_FALSE(geo.isGrid(0, 128));
}

TEST_F(DigitalGeometryTest, Filters) {

  // confirms that they are stacked in x only

  geo.add_bus(bus);

  Filter f;
  f.minimum = 3;
  f.maximum = 7;
  f.rescale_factor = 0.5;

  BusDefinitionStruct bus2;
  bus2.channel_mappings = std::make_shared<MGSeqMappings>();
  bus2.channel_mappings->wire_filters.override_filter(5, f);
  bus2.channel_mappings->grid_filters.override_filter(10, f);
  geo.add_bus(bus2);

  EXPECT_EQ(geo.rescale(0, 5, 10), 10);
  EXPECT_EQ(geo.rescale(1, 4, 10), 10);
  EXPECT_EQ(geo.rescale(1, 5, 10), 5);
  EXPECT_EQ(geo.rescale(1, 6, 10), 10);
  EXPECT_EQ(geo.rescale(1, 90, 10), 5);

  EXPECT_TRUE(geo.is_valid(0, 5, 10));
  EXPECT_TRUE(geo.is_valid(1, 4, 10));
  EXPECT_FALSE(geo.is_valid(1, 5, 10));
  EXPECT_TRUE(geo.is_valid(1, 6, 10));
  EXPECT_FALSE(geo.is_valid(1, 90, 10));
}

TEST_F(DigitalGeometryTest, PrintsSelf) {

  geo.add_bus(bus);
  EXPECT_FALSE(geo.debug().empty());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
