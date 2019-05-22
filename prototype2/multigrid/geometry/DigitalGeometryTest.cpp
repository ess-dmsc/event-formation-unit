/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/TSCTimer.h>
#include <multigrid/geometry/DigitalGeometry.h>
#include <multigrid/geometry/MGSeqGeometry.h>
#include <test/TestBase.h>

using namespace Multigrid;

class DigitalGeometryTest : public TestBase {
protected:
  DigitalGeometry geo;
  std::shared_ptr<MGSeqGeometry> bus;

  virtual void SetUp() {
    bus = std::make_shared<MGSeqGeometry>();
  }
  virtual void TearDown() {
  }
};

TEST_F(DigitalGeometryTest, Nothing) {

  EXPECT_EQ(geo.max_wire(), 0);
  EXPECT_EQ(geo.max_grid(), 0);
  EXPECT_EQ(geo.max_x(), 0);
  EXPECT_EQ(geo.max_y(), 0);
  EXPECT_EQ(geo.max_z(), 0);

  for (int i = 0; i < 120; i++) {
    EXPECT_FALSE(geo.isWire(0, 0, i));
    EXPECT_FALSE(geo.isGrid(0, 0, i));
  }

  for (int i = 0; i < 120; i++) {
    EXPECT_FALSE(geo.isWire(1, 0, i));
    EXPECT_FALSE(geo.isGrid(1, 0, i));
  }

  EXPECT_FALSE(geo.isWire(0, 0, 128));
  EXPECT_FALSE(geo.isGrid(0, 0, 128));
}

TEST_F(DigitalGeometryTest, OneBus) {
  geo.add_bus(bus);

  EXPECT_EQ(geo.max_wire(), 80);
  EXPECT_EQ(geo.max_grid(), 40);
  EXPECT_EQ(geo.max_x(), 4);
  EXPECT_EQ(geo.max_y(), 40);
  EXPECT_EQ(geo.max_z(), 20);

  for (int i = 0; i <= 79; i++) {
    EXPECT_TRUE(geo.isWire(0, 0, i));
    EXPECT_FALSE(geo.isWire(1, 0, i));
    EXPECT_FALSE(geo.isGrid(0, 0, i));
    EXPECT_FALSE(geo.isGrid(1, 0, i));
    EXPECT_EQ(geo.x(0, 0, i), bus->x(0, i));
    EXPECT_EQ(geo.z(0, 0, i), bus->z(0, i));
    EXPECT_EQ(geo.wire(0, 0, i), bus->wire(0, i));
  }

  for (int i = 80; i <= 119; i++) {
    EXPECT_FALSE(geo.isWire(0, 0, i));
    EXPECT_FALSE(geo.isWire(1, 0, i));
    EXPECT_TRUE(geo.isGrid(0, 0, i));
    EXPECT_FALSE(geo.isGrid(1, 0, i));
    EXPECT_EQ(geo.y(0, 0, i), bus->y(0, i));
    EXPECT_EQ(geo.grid(0, 0, i), bus->grid(0, i));
  }

  EXPECT_FALSE(geo.isWire(0, 0, 128));
  EXPECT_FALSE(geo.isGrid(0, 0, 128));
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
    EXPECT_TRUE(geo.isWire(0, 0, i));
    EXPECT_TRUE(geo.isWire(1, 0, i));
    EXPECT_FALSE(geo.isWire(2, 0, i));
    EXPECT_FALSE(geo.isGrid(0, 0, i));
    EXPECT_FALSE(geo.isGrid(1, 0, i));
    EXPECT_FALSE(geo.isGrid(2, 0, i));

    EXPECT_EQ(geo.x(0, 0, i), bus->x(0, i));
    EXPECT_EQ(geo.z(0, 0, i), bus->z(0, i));
    EXPECT_EQ(geo.wire(0, 0, i), bus->wire(0, i));
    EXPECT_EQ(geo.wire(1, 0, i), bus->wire(0, i) + bus->max_wire());

    EXPECT_EQ(geo.x(1, 0, i), bus->x(0, i) + bus->max_x());
    EXPECT_EQ(geo.z(1, 0, i), bus->z(0, i));
    EXPECT_EQ(geo.wire(1, 0, i), bus->wire(0, i) + bus->max_wire());
    EXPECT_EQ(geo.x_from_wire(geo.wire(0, 0, i)), geo.x(0, 0, i));
    EXPECT_EQ(geo.x_from_wire(geo.wire(1, 0, i)), geo.x(1, 0, i));
    EXPECT_EQ(geo.z_from_wire(geo.wire(0, 0, i)), geo.z(0, 0, i));
    EXPECT_EQ(geo.z_from_wire(geo.wire(1, 0, i)), geo.z(1, 0, i));
  }

  for (int i = 80; i <= 119; i++) {
    EXPECT_FALSE(geo.isWire(0, 0, i));
    EXPECT_FALSE(geo.isWire(1, 0, i));
    EXPECT_FALSE(geo.isWire(2, 0, i));
    EXPECT_TRUE(geo.isGrid(0, 0, i));
    EXPECT_TRUE(geo.isGrid(1, 0, i));
    EXPECT_FALSE(geo.isGrid(2, 0, i));

    EXPECT_EQ(geo.grid(0, 0, i), bus->grid(0, i));
    EXPECT_EQ(geo.grid(1, 0, i), bus->grid(0, i) + bus->max_grid());
    EXPECT_EQ(geo.y(0, 0, i), bus->y(0, i));
    EXPECT_EQ(geo.y(1, 0, i), bus->y(0, i));
    EXPECT_EQ(geo.y_from_grid(geo.grid(0, 0, i)), geo.y(0, 0, i));
    EXPECT_EQ(geo.y_from_grid(geo.grid(1, 0, i)), geo.y(1, 0, i));
  }

  EXPECT_FALSE(geo.isWire(0, 0, 128));
  EXPECT_FALSE(geo.isGrid(0, 0, 128));
}

TEST_F(DigitalGeometryTest, Filters) {

  // confirms that they are stacked in x only

  geo.add_bus(bus);


  Filter f;
  f.minimum = 3;
  f.maximum = 7;
  f.rescale_factor = 0.5;

  auto bus2 = std::make_shared<MGSeqGeometry>();
  bus2->override_wire_filter(5, f);
  bus2->override_grid_filter(10, f);
  geo.add_bus(bus2);

  EXPECT_EQ(geo.rescale(0, 0, 5, 10), 10);
  EXPECT_EQ(geo.rescale(1, 0, 4, 10), 10);
  EXPECT_EQ(geo.rescale(1, 0, 5, 10), 5);
  EXPECT_EQ(geo.rescale(1, 0, 6, 10), 10);
  EXPECT_EQ(geo.rescale(1, 0, 90, 10), 5);

  EXPECT_TRUE(geo.is_valid(0, 0, 5, 10));
  EXPECT_TRUE(geo.is_valid(1, 0, 4, 10));
  EXPECT_FALSE(geo.is_valid(1, 0, 5, 10));
  EXPECT_TRUE(geo.is_valid(1, 0, 6, 10));
  EXPECT_FALSE(geo.is_valid(1, 0, 90, 10));
}

TEST_F(DigitalGeometryTest, PrintsSelf) {

  geo.add_bus(bus);
  EXPECT_FALSE(geo.debug().empty());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
