/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/geometry/DetectorMappings.h>
#include <multigrid/geometry/MG24Mappings.h>
#include <multigrid/geometry/MGSeqMappings.h>
#include <multigrid/geometry/PlaneMappings.h>

#include <common/testutils/TestBase.h>

using namespace Multigrid;

class DetectorMappingsTest : public TestBase {
protected:
  DetectorMappings mapping;
  std::shared_ptr<ChannelMappings> bus, bus2;
  Hit hit;
  uint8_t invalid_plane{Hit::InvalidPlane};
  uint16_t invalid_coordinate{Hit::InvalidCoord};
  uint8_t wire_plane{Multigrid::wire_plane};
  uint8_t grid_plane{Multigrid::grid_plane};

  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DetectorMappingsTest, Nothing) {

  EXPECT_EQ(mapping.max_wire(), 0);
  EXPECT_EQ(mapping.max_grid(), 0);

  for (int i = 0; i < 3000; i++) {
    EXPECT_FALSE(mapping.map(hit, 0, i, 0));
    EXPECT_EQ(hit.plane, invalid_plane);
    EXPECT_EQ(hit.coordinate, invalid_coordinate);
  }

  for (int i = 0; i < 3000; i++) {
    EXPECT_FALSE(mapping.map(hit, 1, i, 0));
    EXPECT_EQ(hit.plane, invalid_plane);
    EXPECT_EQ(hit.coordinate, invalid_coordinate);
  }
}

TEST_F(DetectorMappingsTest, OneBus) {
  bus = std::make_shared<MGSeqMappings>();
  mapping.add_bus(bus);

  EXPECT_EQ(mapping.max_wire(), 80);
  EXPECT_EQ(mapping.max_grid(), 40);

  for (int i = 0; i <= 79; i++) {
    EXPECT_TRUE(mapping.map(hit, 0, i, 0));
    EXPECT_EQ(hit.plane, wire_plane);
    EXPECT_EQ(hit.coordinate, bus->wire(i));

    EXPECT_FALSE(mapping.map(hit, 1, i, 0));
    EXPECT_EQ(hit.plane, invalid_plane);
    EXPECT_EQ(hit.coordinate, invalid_coordinate);
  }

  for (int i = 80; i <= 119; i++) {
    EXPECT_TRUE(mapping.map(hit, 0, i, 0));
    EXPECT_EQ(hit.plane, grid_plane);
    EXPECT_EQ(hit.coordinate, bus->grid(i));

    EXPECT_FALSE(mapping.map(hit, 1, i, 0));
    EXPECT_EQ(hit.plane, invalid_plane);
    EXPECT_EQ(hit.coordinate, invalid_coordinate);
  }

  EXPECT_FALSE(mapping.map(hit, 0, 128, 0));
  EXPECT_EQ(hit.plane, invalid_plane);
}

TEST_F(DetectorMappingsTest, TwoBuses) {
  bus = std::make_shared<MGSeqMappings>();
  bus2 = std::make_shared<MGSeqMappings>();

  // confirms that they are stacked in x only

  mapping.add_bus(bus);
  mapping.add_bus(bus2);

  EXPECT_EQ(mapping.max_wire(), 160);
  EXPECT_EQ(mapping.max_grid(), 80);

  for (int i = 0; i <= 79; i++) {
    EXPECT_TRUE(mapping.map(hit, 0, i, 0));
    EXPECT_EQ(hit.plane, wire_plane);
    EXPECT_EQ(hit.coordinate, bus->wire(i));

    EXPECT_TRUE(mapping.map(hit, 1, i, 0));
    EXPECT_EQ(hit.plane, 2 + wire_plane);
    EXPECT_EQ(hit.coordinate, bus2->wire(i));
    hit = mapping.absolutify(hit);
    EXPECT_EQ(hit.plane, wire_plane);
    EXPECT_EQ(hit.coordinate, bus2->wire(i) + bus->max_wire());

    EXPECT_FALSE(mapping.map(hit, 2, i, 0));
    EXPECT_EQ(hit.plane, invalid_plane);
    EXPECT_EQ(hit.coordinate, invalid_coordinate);
  }

  for (int i = 80; i <= 119; i++) {
    EXPECT_TRUE(mapping.map(hit, 0, i, 0));
    hit = mapping.absolutify(hit);
    EXPECT_EQ(hit.plane, grid_plane);
    EXPECT_EQ(hit.coordinate, bus->grid(i));

    EXPECT_TRUE(mapping.map(hit, 1, i, 0));
    EXPECT_EQ(hit.plane, 2 + grid_plane);
    EXPECT_EQ(hit.coordinate, bus2->grid(i));
    hit = mapping.absolutify(hit);
    EXPECT_EQ(hit.plane, grid_plane);
    EXPECT_EQ(hit.coordinate, bus2->grid(i) + bus->max_grid());

    EXPECT_FALSE(mapping.map(hit, 2, i, 0));
    EXPECT_EQ(hit.plane, invalid_plane);
    EXPECT_EQ(hit.coordinate, invalid_coordinate);
  }
}

// \todo test with MG24 and other mixed module configurations

TEST_F(DetectorMappingsTest, Filters) {
  bus = std::make_shared<MGSeqMappings>();
  mapping.add_bus(bus);

  Filter f;
  f.minimum = 3;
  f.maximum = 7;
  f.rescale_factor = 0.5;

  bus2 = std::make_shared<MGSeqMappings>();
  bus2->wire_filters.override_filter(5, f);
  bus2->grid_filters.override_filter(10, f);
  mapping.add_bus(bus2);

  EXPECT_TRUE(mapping.map(hit, 0, 5, 10));
  EXPECT_EQ(hit.weight, 10);

  EXPECT_TRUE(mapping.map(hit, 1, 4, 10));
  EXPECT_EQ(hit.weight, 10);

  EXPECT_TRUE(mapping.map(hit, 1, 5, 10));
  EXPECT_EQ(hit.weight, 5);

  EXPECT_TRUE(mapping.map(hit, 1, 6, 10));
  EXPECT_EQ(hit.weight, 10);

  EXPECT_TRUE(mapping.map(hit, 1, 90, 10));
  EXPECT_EQ(hit.weight, 5);
}

TEST_F(DetectorMappingsTest, PrintsSelf) {
  bus = std::make_shared<MGSeqMappings>();
  mapping.add_bus(bus);
  EXPECT_FALSE(mapping.debug().empty());
  MESSAGE() << mapping.debug() << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
