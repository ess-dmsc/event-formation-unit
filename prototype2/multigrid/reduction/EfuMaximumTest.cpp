/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/reduction/EfuMaximum.h>
#include <test/TestBase.h>

using namespace Multigrid;

class EfuMaximumTest : public TestBase {
protected:
  EfuMaximum efu;
  virtual void SetUp() {
    efu.mappings.add_bus(BusGeometry());
  }
  virtual void TearDown() {
  }
};

TEST_F(EfuMaximumTest, DefaultConstructed) {
  EXPECT_FALSE(efu.event_good());
}

TEST_F(EfuMaximumTest, WireOnly) {
  MesytecReadout wire_hit;
  wire_hit.channel = 7;
  EXPECT_TRUE(efu.mappings.isWire(wire_hit.bus, wire_hit.channel));
  efu.ingest(wire_hit);
  EXPECT_FALSE(efu.event_good());
  EXPECT_EQ(efu.x(), efu.mappings.x(wire_hit.bus, wire_hit.channel));
  EXPECT_EQ(efu.z(), efu.mappings.z(wire_hit.bus, wire_hit.channel));
}

TEST_F(EfuMaximumTest, GridOnly) {
  MesytecReadout grid_hit;
  grid_hit.channel = 90;
  EXPECT_TRUE(efu.mappings.isGrid(grid_hit.bus, grid_hit.channel));
  efu.ingest(grid_hit);
  EXPECT_FALSE(efu.event_good());
  EXPECT_EQ(efu.y(), efu.mappings.y(grid_hit.bus, grid_hit.channel));
}

TEST_F(EfuMaximumTest, WireAndGrid) {
  MesytecReadout wire_hit;
  wire_hit.channel = 7;
  EXPECT_TRUE(efu.mappings.isWire(wire_hit.bus, wire_hit.channel));
  efu.ingest(wire_hit);

  MesytecReadout grid_hit;
  grid_hit.channel = 90;
  EXPECT_TRUE(efu.mappings.isGrid(grid_hit.bus, grid_hit.channel));
  efu.ingest(grid_hit);

  EXPECT_TRUE(efu.event_good());

  EXPECT_EQ(efu.x(), efu.mappings.x(wire_hit.bus, wire_hit.channel));
  EXPECT_EQ(efu.y(), efu.mappings.y(grid_hit.bus, grid_hit.channel));
  EXPECT_EQ(efu.z(), efu.mappings.z(wire_hit.bus, wire_hit.channel));
}

TEST_F(EfuMaximumTest, HighestAdcWire) {
  MesytecReadout hit1;
  hit1.channel = 7;
  efu.ingest(hit1);
  EXPECT_EQ(efu.x(), efu.mappings.x(hit1.bus, hit1.channel));

  MesytecReadout hit2;
  hit2.channel = 9;
  hit2.adc = 16;
  efu.ingest(hit2);
  EXPECT_EQ(efu.x(), efu.mappings.x(hit2.bus, hit2.channel));


  MesytecReadout hit3;
  hit3.channel = 50;
  hit3.adc = 8;
  efu.ingest(hit3);
  EXPECT_EQ(efu.x(), efu.mappings.x(hit2.bus, hit2.channel));
}

TEST_F(EfuMaximumTest, HighestAdcGrid) {
  MesytecReadout hit1;
  hit1.channel = 97;
  efu.ingest(hit1);
  EXPECT_EQ(efu.y(), efu.mappings.y(hit1.bus, hit1.channel));

  MesytecReadout hit2;
  hit2.channel = 99;
  hit2.adc = 16;
  efu.ingest(hit2);
  EXPECT_EQ(efu.y(), efu.mappings.y(hit2.bus, hit2.channel));


  MesytecReadout hit3;
  hit3.channel = 99;
  hit3.adc = 8;
  efu.ingest(hit3);
  EXPECT_EQ(efu.y(), efu.mappings.y(hit2.bus, hit2.channel));
}

/// \todo time

/// \todo hists

/// \todo serializer

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
