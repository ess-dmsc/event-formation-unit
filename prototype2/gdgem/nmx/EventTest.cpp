/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cmath>
#include <gdgem/nmx/Event.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

using namespace Gem;

class EventTest : public TestBase {
protected:
  Hit e;
  Event event;
  virtual void SetUp() {  }
  virtual void TearDown() {  }
};

// \todo reuse only the utpc tests

TEST_F(EventTest, Insert) {
  event.insert_hit(e);
  EXPECT_EQ(event.x.hit_count(), 1);
  e.plane = 1;
  event.insert_hit(e);
  EXPECT_EQ(event.y.hit_count(), 1);
}

TEST_F(EventTest, Empty) {
  EXPECT_TRUE(event.empty());
  event.insert_hit(e);
  EXPECT_FALSE(event.empty());
}

TEST_F(EventTest, Merge) {
  Cluster x;
  e.plane = 0;
  x.insert(e);
  x.insert(e);
  event.merge(x);
  EXPECT_FALSE(event.empty());
  EXPECT_EQ(event.x.hit_count(), 2);
}

TEST_F(EventTest, MergeTwice) {
  Cluster x;
  e.plane = 0;

  x.insert(e);
  x.insert(e);
  event.merge(x);
  EXPECT_EQ(event.x.hit_count(), 2);

  x.clear();
  x.insert(e);
  x.insert(e);
  x.insert(e);
  event.merge(x);
  EXPECT_EQ(event.x.hit_count(), 5);
}

TEST_F(EventTest, MergeXY) {
  Cluster x, y;

  e.plane = 0;
  x.insert(e);
  x.insert(e);
  event.merge(x);

  e.plane = 1;
  y.insert(e);
  y.insert(e);
  y.insert(e);
  event.merge(y);
  EXPECT_EQ(event.x.hit_count(), 2);
  EXPECT_EQ(event.y.hit_count(), 3);
}

TEST_F(EventTest, TimeSpan) {
  Cluster x, y;

  e.plane = 0;
  e.time = 3; x.insert(e);
  e.time = 7; x.insert(e);
  event.merge(x);

  e.plane = 1;
  e.time = 5; y.insert(e);
  e.time = 1; y.insert(e);
  event.merge(y);
  EXPECT_EQ(event.time_end(), 7);
  EXPECT_EQ(event.time_start(), 1);
  EXPECT_EQ(event.time_span(), 6);
}


TEST_F(EventTest, AnalyzeBadY) {
  event.insert_hit(e);
//  auto result_x = utpcAnalyzer(true, 5, 5).analyze(event.x);
//  auto result_y = utpcAnalyzer(true, 5, 5).analyze(event.y);

  EXPECT_FALSE(event.valid());
}

TEST_F(EventTest, AnalyzeBadX) {
  e.plane = 1;
  event.insert_hit(e);
//  auto result_x = utpcAnalyzer(true, 5, 5).analyze(event.x);
//  auto result_y = utpcAnalyzer(true, 5, 5).analyze(event.y);

  EXPECT_FALSE(event.valid());
}

TEST_F(EventTest, AnalyzeGood) {
  event.insert_hit(e);
  e.plane = 1;
  event.insert_hit(e);
//  auto result_x = utpcAnalyzer(true, 5, 5).analyze(event.x);
//  auto result_y = utpcAnalyzer(true, 5, 5).analyze(event.y);

  EXPECT_TRUE(event.valid());
}

TEST_F(EventTest, InsertInvalid) {
  e.plane = 0;
  event.insert_hit(e);
  e.plane = 1;
  event.insert_hit(e);
  e.plane = 2;
  event.insert_hit(e);
  EXPECT_EQ(2, event.x.hit_count() + event.y.hit_count());
}

TEST_F(EventTest, DebugPrint) {
  MESSAGE() << "This is not a test, just calling debug print function\n";
  event.insert_hit(e);
  e.plane = 1;
  event.insert_hit(e);

//  auto result_x = utpcAnalyzer(true, 5, 5).analyze(event.x);
//  auto result_y = utpcAnalyzer(true, 5, 5).analyze(event.y);

  auto debugstr = event.debug();
  MESSAGE() << debugstr << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
