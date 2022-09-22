/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/testutils/TestBase.h>
#include <multigrid/geometry/MGSeqMappings.h>

using namespace Multigrid;

class MGSeqMappingsTest : public TestBase {
protected:
  MGSeqMappings mappings;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MGSeqMappingsTest, DefaultConstructedValues) {
  EXPECT_EQ(mappings.max_channel(), 120);
  EXPECT_EQ(mappings.max_wire(), 80);
  EXPECT_FALSE(mappings.swap_wires());
  EXPECT_FALSE(mappings.swap_grids());
}

TEST_F(MGSeqMappingsTest, WireLimits) {
  mappings.max_wire(40);
  EXPECT_EQ(mappings.max_wire(), 40);
  EXPECT_TRUE(mappings.isWire(0));
  EXPECT_TRUE(mappings.isWire(2));
  EXPECT_TRUE(mappings.isWire(39));
  EXPECT_FALSE(mappings.isWire(40));
  EXPECT_FALSE(mappings.isWire(7000));
}

TEST_F(MGSeqMappingsTest, GridLimits) {
  mappings.max_channel(100);
  EXPECT_EQ(mappings.max_channel(), 100);
  EXPECT_FALSE(mappings.isGrid(0));
  EXPECT_FALSE(mappings.isGrid(79));
  EXPECT_TRUE(mappings.isGrid(80));
  EXPECT_TRUE(mappings.isGrid(99));
  EXPECT_FALSE(mappings.isGrid(100));
  EXPECT_FALSE(mappings.isWire(100));
}

TEST_F(MGSeqMappingsTest, GetWire) {
  mappings.max_wire(40);
  EXPECT_EQ(mappings.max_wire(), 40);
  EXPECT_EQ(mappings.wire(0), 0);
  EXPECT_EQ(mappings.wire(2), 2);
  EXPECT_EQ(mappings.wire(39), 39);

  // undefined for out of bounds
}

TEST_F(MGSeqMappingsTest, SwapWires) {
  mappings.swap_wires(true);
  EXPECT_TRUE(mappings.swap_wires());
  EXPECT_EQ(mappings.max_wire(), 80);

  EXPECT_EQ(mappings.wire(0), 1);
  EXPECT_EQ(mappings.wire(1), 0);
  EXPECT_EQ(mappings.wire(39), 38);
  EXPECT_EQ(mappings.wire(38), 39);

  // undefined for out of bounds
}

TEST_F(MGSeqMappingsTest, GetGrid) {
  mappings.max_channel(100);
  EXPECT_EQ(mappings.grid(80), 0);
  EXPECT_EQ(mappings.grid(99), 19);

  // undefined for out of bounds
}

TEST_F(MGSeqMappingsTest, SwapGrids) {
  mappings.swap_grids(true);
  EXPECT_TRUE(mappings.swap_grids());
  EXPECT_EQ(mappings.max_channel(), 120);

  EXPECT_EQ(mappings.grid(80), 1);
  EXPECT_EQ(mappings.grid(81), 0);
  EXPECT_EQ(mappings.grid(98), 19);
  EXPECT_EQ(mappings.grid(99), 18);

  // undefined for out of bounds
}

TEST_F(MGSeqMappingsTest, PrintsSelf) {
  mappings.swap_wires(true);
  mappings.swap_grids(true);
  EXPECT_FALSE(mappings.debug({}).empty());
}

TEST_F(MGSeqMappingsTest, FromJsonMinimal) {
  nlohmann::json j;
  j["max_channel"] = 100;
  j["max_wire"] = 50;

  mappings = j;

  //  EXPECT_EQ(mappings.max_x(), 10);
  //  EXPECT_EQ(mappings.max_y(), 50);
  //  EXPECT_EQ(mappings.max_z(), 5);
}

TEST_F(MGSeqMappingsTest, FromJson) {
  nlohmann::json j;
  j["max_channel"] = 100;
  j["max_wire"] = 50;

  j["swap_wires"] = true;
  j["swap_grids"] = true;

  nlohmann::json b;
  b["min"] = 3;
  b["max"] = 7;
  b["rescale"] = 0.5;
  b["count"] = 10;
  j["wire_filters"]["blanket"] = b;

  nlohmann::json j2;
  j2["idx"] = 5;
  j2["min"] = 3;
  j2["max"] = 7;
  j2["rescale"] = 0.5;
  j["wire_filters"]["exceptions"].push_back(j2);

  mappings = j;

  // \todo test correct parsing
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
