/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cmath>
#include <gdgem/nmx/Event.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class EventTest : public TestBase {
protected:
  Eventlet e;
  Event event;
  virtual void SetUp() {  }
  virtual void TearDown() {  }
};

TEST_F(EventTest, Insert) {
  event.insert_eventlet(e);
  EXPECT_EQ(event.x.entries.size(), 1);
  e.plane_id = 1;
  event.insert_eventlet(e);
  EXPECT_EQ(event.y.entries.size(), 1);
}

TEST_F(EventTest, Empty) {
  EXPECT_TRUE(event.empty());
  event.insert_eventlet(e);
  EXPECT_FALSE(event.empty());
}

TEST_F(EventTest, Merge) {
  Cluster x;
  e.plane_id = 0;
  x.insert_eventlet(e);
  x.insert_eventlet(e);
  event.merge(x);
  EXPECT_FALSE(event.empty());
  EXPECT_EQ(x.entries.size(), 0);
  EXPECT_EQ(event.x.entries.size(), 2);
}

TEST_F(EventTest, MergeTwice) {
  Cluster x;
  e.plane_id = 0;

  x.insert_eventlet(e);
  x.insert_eventlet(e);
  event.merge(x);
  EXPECT_EQ(event.x.entries.size(), 2);

  x.insert_eventlet(e);
  x.insert_eventlet(e);
  x.insert_eventlet(e);
  event.merge(x);
  EXPECT_EQ(event.x.entries.size(), 5);
}

TEST_F(EventTest, MergeXY) {
  Cluster x, y;

  e.plane_id = 0;
  x.insert_eventlet(e);
  x.insert_eventlet(e);
  event.merge(x);

  e.plane_id = 1;
  y.insert_eventlet(e);
  y.insert_eventlet(e);
  y.insert_eventlet(e);
  event.merge(y);
  EXPECT_EQ(event.x.entries.size(), 2);
  EXPECT_EQ(event.y.entries.size(), 3);
}

TEST_F(EventTest, TimeSpan) {
  Cluster x, y;

  e.plane_id = 0;
  e.time = 3; x.insert_eventlet(e);
  e.time = 7; x.insert_eventlet(e);
  event.merge(x);

  e.plane_id = 1;
  e.time = 5; y.insert_eventlet(e);
  e.time = 1; y.insert_eventlet(e);
  event.merge(y);
  EXPECT_EQ(event.time_end(), 7);
  EXPECT_EQ(event.time_start(), 1);
  EXPECT_EQ(event.time_span(), 6);
}


TEST_F(EventTest, AnalyzeBadY) {
  event.insert_eventlet(e);
  event.analyze(true, 5, 5);
  EXPECT_FALSE(event.valid());
}

TEST_F(EventTest, AnalyzeBadX) {
  e.plane_id = 1;
  event.insert_eventlet(e);
  event.analyze(true, 5, 5);
  EXPECT_FALSE(event.valid());
}

TEST_F(EventTest, AnalyzeGood) {
  event.insert_eventlet(e);
  e.plane_id = 1;
  event.insert_eventlet(e);
  event.analyze(true, 5, 5);
  EXPECT_TRUE(event.valid());
}

TEST_F(EventTest, InsertInvalid) {
  e.plane_id = 0;
  event.insert_eventlet(e);
  e.plane_id = 1;
  event.insert_eventlet(e);
  e.plane_id = 2;
  event.insert_eventlet(e);
  EXPECT_EQ(2, event.x.entries.size() + event.y.entries.size());
}

TEST_F(EventTest, DebugPrint) {
  MESSAGE() << "This is not a test, just calling debug print function\n";
  event.insert_eventlet(e);
  e.plane_id = 1;
  event.insert_eventlet(e);
  event.analyze(true, 5, 5);
  auto debugstr = event.debug();
  MESSAGE() << debugstr << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
