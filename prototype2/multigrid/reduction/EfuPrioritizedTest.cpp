/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/reduction/EfuPrioritized.h>
#include <test/TestBase.h>

using namespace Multigrid;

class EfuPrioritizedTest : public TestBase {
protected:
  EfuPrioritized efu;
  std::vector<MesytecReadout> hits;
  virtual void SetUp() {
    efu.mappings.add_bus(BusGeometry());
  }
  virtual void TearDown() {
  }
};

TEST_F(EfuPrioritizedTest, DefaultConstructed) {
  EXPECT_FALSE(efu.event_good());
}

TEST_F(EfuPrioritizedTest, WireOnly) {
  MesytecReadout wire_hit;
  wire_hit.adc = 10;
  wire_hit.channel = 7;
  EXPECT_TRUE(efu.mappings.isWire(wire_hit.bus, wire_hit.channel));
  hits.push_back(wire_hit);
  efu.ingest(hits);
  EXPECT_FALSE(efu.event_good());
  EXPECT_EQ(efu.x(), efu.mappings.x(wire_hit.bus, wire_hit.channel));
  EXPECT_EQ(efu.z(), efu.mappings.z(wire_hit.bus, wire_hit.channel));
}

TEST_F(EfuPrioritizedTest, GridOnly) {
  MesytecReadout grid_hit;
  grid_hit.adc = 10;
  grid_hit.channel = 90;
  EXPECT_TRUE(efu.mappings.isGrid(grid_hit.bus, grid_hit.channel));
  hits.push_back(grid_hit);
  efu.ingest(hits);
  EXPECT_FALSE(efu.event_good());
  EXPECT_EQ(efu.y(), efu.mappings.y(grid_hit.bus, grid_hit.channel));
}

TEST_F(EfuPrioritizedTest, WireAndGrid) {
  MesytecReadout wire_hit;
  wire_hit.adc = 10;
  wire_hit.channel = 7;
  EXPECT_TRUE(efu.mappings.isWire(wire_hit.bus, wire_hit.channel));
  hits.push_back(wire_hit);

  MesytecReadout grid_hit;
  grid_hit.adc = 10;
  grid_hit.channel = 90;
  EXPECT_TRUE(efu.mappings.isGrid(grid_hit.bus, grid_hit.channel));
  hits.push_back(grid_hit);

  efu.ingest(hits);

  ASSERT_TRUE(efu.event_good());

  EXPECT_EQ(efu.x(), efu.mappings.x(wire_hit.bus, wire_hit.channel));
  EXPECT_EQ(efu.y(), efu.mappings.y(grid_hit.bus, grid_hit.channel));
  EXPECT_EQ(efu.z(), efu.mappings.z(wire_hit.bus, wire_hit.channel));
}

TEST_F(EfuPrioritizedTest, HighestAdcWire) {
  MesytecReadout hit1;
  hit1.channel = 10;
  hit1.adc = 10;
  hits.push_back(hit1);
  efu.ingest(hits);
  EXPECT_EQ(efu.x(), efu.mappings.x(hit1.bus, hit1.channel));

  MesytecReadout hit2;
  hit2.channel = 25;
  hit2.adc = 5;
  hits.push_back(hit2);
  efu.ingest(hits);
  EXPECT_EQ(efu.x(), 0);

  MesytecReadout hit3;
  hit3.channel = 50;
  hit3.adc = 20;
  hits.push_back(hit3);
  efu.ingest(hits);
  EXPECT_EQ(efu.x(), efu.mappings.x(hit3.bus, hit3.channel));
}

TEST_F(EfuPrioritizedTest, HighestAdcGrid) {
  MesytecReadout hit1;
  hit1.adc = 10;
  hit1.channel = 90;
  hits.push_back(hit1);
  efu.ingest(hits);
  EXPECT_EQ(efu.y(), efu.mappings.y(hit1.bus, hit1.channel));

  MesytecReadout hit2;
  hit2.adc = 10;
  hit2.channel = 100;
  hits.push_back(hit2);
  efu.ingest(hits);
  EXPECT_EQ(efu.y(), 15);


  MesytecReadout hit3;
  hit3.adc = 20;
  hit3.channel = 110;
  hits.push_back(hit3);
  efu.ingest(hits);
  EXPECT_EQ(efu.y(), efu.mappings.y(hit3.bus, hit3.channel));
}

// \todo time

// \todo hists

// \todo serializer

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
