/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/EfuCenterMass.h>
#include <test/TestBase.h>

using namespace Multigrid;

class EfuCenterMassTest : public TestBase {
protected:
  EfuCenterMass efu;
  virtual void SetUp() {
    efu.mappings.add_bus(BusGeometry());
  }
  virtual void TearDown() {
  }
};

TEST_F(EfuCenterMassTest, DefaultConstructed) {
  EXPECT_FALSE(efu.event_good());
}

TEST_F(EfuCenterMassTest, WireOnly) {
  Hit wire_hit;
  wire_hit.adc = 10;
  wire_hit.channel = 7;
  EXPECT_TRUE(efu.mappings.isWire(wire_hit.bus, wire_hit.channel));
  efu.ingest(wire_hit);
  EXPECT_FALSE(efu.event_good());
  EXPECT_EQ(efu.x(), efu.mappings.x(wire_hit.bus, wire_hit.channel));
  EXPECT_EQ(efu.z(), efu.mappings.z(wire_hit.bus, wire_hit.channel));
}

TEST_F(EfuCenterMassTest, GridOnly) {
  Hit grid_hit;
  grid_hit.adc = 10;
  grid_hit.channel = 90;
  EXPECT_TRUE(efu.mappings.isGrid(grid_hit.bus, grid_hit.channel));
  efu.ingest(grid_hit);
  EXPECT_FALSE(efu.event_good());
  EXPECT_EQ(efu.y(), efu.mappings.y(grid_hit.bus, grid_hit.channel));
}

TEST_F(EfuCenterMassTest, WireAndGrid) {
  Hit wire_hit;
  wire_hit.adc = 10;
  wire_hit.channel = 7;
  EXPECT_TRUE(efu.mappings.isWire(wire_hit.bus, wire_hit.channel));
  efu.ingest(wire_hit);

  Hit grid_hit;
  grid_hit.adc = 10;
  grid_hit.channel = 90;
  EXPECT_TRUE(efu.mappings.isGrid(grid_hit.bus, grid_hit.channel));
  efu.ingest(grid_hit);

  ASSERT_TRUE(efu.event_good());

  EXPECT_EQ(efu.x(), efu.mappings.x(wire_hit.bus, wire_hit.channel));
  EXPECT_EQ(efu.y(), efu.mappings.y(grid_hit.bus, grid_hit.channel));
  EXPECT_EQ(efu.z(), efu.mappings.z(wire_hit.bus, wire_hit.channel));
}

TEST_F(EfuCenterMassTest, HighestAdcWire) {
  Hit hit1;
  hit1.channel = 10;
  hit1.adc = 10;
  efu.ingest(hit1);
  EXPECT_EQ(efu.x(), efu.mappings.x(hit1.bus, hit1.channel));

  Hit hit2;
  hit2.channel = 25;
  hit2.adc = 10;
  efu.ingest(hit2);
  EXPECT_EQ(efu.x(), 0);


  Hit hit3;
  hit3.channel = 50;
  hit3.adc = 20;
  efu.ingest(hit3);
  EXPECT_EQ(efu.x(), efu.mappings.x(hit2.bus, hit2.channel));
}

TEST_F(EfuCenterMassTest, HighestAdcGrid) {
  Hit hit1;
  hit1.adc = 10;
  hit1.channel = 90;
  efu.ingest(hit1);
  EXPECT_EQ(efu.y(), efu.mappings.y(hit1.bus, hit1.channel));

  Hit hit2;
  hit2.channel = 100;
  hit2.adc = 10;
  efu.ingest(hit2);
  EXPECT_EQ(efu.y(), 15);


  Hit hit3;
  hit3.channel = 110;
  hit3.adc = 20;
  efu.ingest(hit3);
  EXPECT_EQ(efu.y(), 22);
}

/// \todo time

/// \todo hists

/// \todo serializer

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
