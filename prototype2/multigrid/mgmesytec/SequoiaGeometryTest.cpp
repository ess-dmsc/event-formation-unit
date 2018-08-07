/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <libs/include/TSCTimer.h>
#include <multigrid/mgmesytec/SequoiaGeometry.h>
#include <test/TestBase.h>

using namespace Multigrid;

class SequoiaGeometryTest : public TestBase {
protected:
  SequoiaGeometry geo;
  BusGeometry bus;

  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
};

TEST_F(SequoiaGeometryTest, Nothing) {

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


TEST_F(SequoiaGeometryTest, OneBus) {
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
    EXPECT_EQ(geo.x(0,i), bus.x(i));
    EXPECT_EQ(geo.z(0,i), bus.z(i));
  }

  for (int i = 80; i <= 119; i++) {
    EXPECT_FALSE(geo.isWire(0, i));
    EXPECT_FALSE(geo.isWire(1, i));
    EXPECT_TRUE(geo.isGrid(0, i));
    EXPECT_FALSE(geo.isGrid(1, i));
    EXPECT_EQ(geo.y(0,i), bus.y(i));
  }

  EXPECT_FALSE(geo.isWire(0, 128));
  EXPECT_FALSE(geo.isGrid(0, 128));
}

TEST_F(SequoiaGeometryTest, TwoBuses) {

  // confirms that they are stacked in x only

  geo.add_bus(bus);
  geo.add_bus(bus);

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

    EXPECT_EQ(geo.x(0,i), bus.x(i));
    EXPECT_EQ(geo.z(0,i), bus.z(i));

    EXPECT_EQ(geo.x(1,i), bus.x(i) + bus.max_x());
    EXPECT_EQ(geo.z(1,i), bus.z(i));
  }

  for (int i = 80; i <= 119; i++) {
    EXPECT_FALSE(geo.isWire(0, i));
    EXPECT_FALSE(geo.isWire(1, i));
    EXPECT_FALSE(geo.isWire(2, i));
    EXPECT_TRUE(geo.isGrid(0, i));
    EXPECT_TRUE(geo.isGrid(1, i));
    EXPECT_FALSE(geo.isGrid(2, i));

    EXPECT_EQ(geo.y(0,i), bus.y(i));
    EXPECT_EQ(geo.y(1,i), bus.y(i));
  }

  EXPECT_FALSE(geo.isWire(0, 128));
  EXPECT_FALSE(geo.isGrid(0, 128));
}

TEST_F(SequoiaGeometryTest, Filters) {

  // confirms that they are stacked in x only

  geo.add_bus(bus);

  Filter f;
  f.minimum = 3;
  f.maximum = 7;
  f.rescale_factor = 0.5;
  bus.override_wire_filter(5, f);
  geo.add_bus(bus);

  EXPECT_EQ(geo.rescale(0, 5, 10), 10);
  EXPECT_EQ(geo.rescale(1, 4, 10), 10);
  EXPECT_EQ(geo.rescale(1, 5, 10), 5);
  EXPECT_EQ(geo.rescale(1, 6, 10), 10);

  EXPECT_TRUE(geo.is_valid(0, 5, 10));
  EXPECT_TRUE(geo.is_valid(1, 4, 10));
  EXPECT_FALSE(geo.is_valid(1, 5, 10));
  EXPECT_TRUE(geo.is_valid(1, 6, 10));
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
