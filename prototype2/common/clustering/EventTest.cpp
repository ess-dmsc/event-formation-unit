/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cmath>
#include <common/clustering/Event.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class EventTest : public TestBase {
protected:
  Event event;
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(EventTest, Insert) {
  event.insert_hit({0, 0, 0, 0});
  EXPECT_EQ(event.c1.hit_count(), 1);
  event.insert_hit({0, 1, 0, 0});
  EXPECT_EQ(event.c2.hit_count(), 1);
}

TEST_F(EventTest, Empty) {
  EXPECT_TRUE(event.empty());
  event.insert_hit({0, 0, 0, 0});
  EXPECT_FALSE(event.empty());
}

TEST_F(EventTest, Merge) {
  Cluster x;
  x.insert_hit({0, 0, 0, 0});
  x.insert_hit({0, 0, 0, 0});
  event.merge(x);
  EXPECT_FALSE(event.empty());
  EXPECT_EQ(x.hit_count(), 0);
  EXPECT_EQ(event.c1.hit_count(), 2);
}

TEST_F(EventTest, MergeTwice) {
  Cluster x;
  x.insert_hit({0, 0, 0, 0});
  x.insert_hit({0, 0, 0, 0});
  event.merge(x);
  EXPECT_EQ(event.c1.hit_count(), 2);

  x.insert_hit({0, 0, 0, 0});
  x.insert_hit({0, 0, 0, 0});
  x.insert_hit({0, 0, 0, 0});
  event.merge(x);
  EXPECT_EQ(event.c1.hit_count(), 5);
}

TEST_F(EventTest, MergeXY) {
  Cluster x, y;

  x.insert_hit({0, 0, 0, 0});
  x.insert_hit({0, 0, 0, 0});
  event.merge(x);

  y.insert_hit({0, 1, 0, 0});
  y.insert_hit({0, 1, 0, 0});
  y.insert_hit({0, 1, 0, 0});
  event.merge(y);
  EXPECT_EQ(event.c1.hit_count(), 2);
  EXPECT_EQ(event.c2.hit_count(), 3);
}

TEST_F(EventTest, TimeSpanEmpty) {
  EXPECT_EQ(event.time_span(), 0);
}

TEST_F(EventTest, TimeSpanXOnly) {
  Cluster x;

  x.insert_hit({3, 0, 0, 0});
  x.insert_hit({7, 0, 0, 0});
  event.merge(x);

  EXPECT_EQ(event.time_end(), 7);
  EXPECT_EQ(event.time_start(), 3);
  EXPECT_EQ(event.time_span(), 5);
}

TEST_F(EventTest, TimeSpanYOnly) {
  Cluster y;

  y.insert_hit({5, 1, 0, 0});
  y.insert_hit({1, 1, 0, 0});
  event.merge(y);
  EXPECT_EQ(event.time_end(), 5);
  EXPECT_EQ(event.time_start(), 1);
  EXPECT_EQ(event.time_span(), 5);
}

TEST_F(EventTest, TimeSpan) {
  Cluster x, y;

  x.insert_hit({3, 0, 0, 0});
  x.insert_hit({7, 0, 0, 0});
  event.merge(x);

  y.insert_hit({5, 1, 0, 0});
  y.insert_hit({1, 1, 0, 0});
  event.merge(y);
  EXPECT_EQ(event.time_end(), 7);
  EXPECT_EQ(event.time_start(), 1);
  EXPECT_EQ(event.time_span(), 7);
}

TEST_F(EventTest, IgnoreInvalidPlane) {
  event = Event(1,3);

  event.insert_hit({0, 0, 0, 0});
  event.insert_hit({5, 1, 0, 0});
  event.insert_hit({10, 2, 0, 0});
  event.insert_hit({50, 3, 0, 0});
  EXPECT_EQ(event.time_span(), 46);
}

TEST_F(EventTest, DebugPrint) {
  event.insert_hit({7,0,5,1});
  event.insert_hit({0,1,5,1});

  MESSAGE() << "NOT A UNIT TEST: please manually check output\n";
  MESSAGE() << "SIMPLE:\n" << event.debug() << "\n";
  MESSAGE() << "VERBOSE:\n" << event.debug(true) << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
