/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cmath>
#include <common/clustering/Event.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class EventTest : public TestBase {
protected:
  Event event;
};

TEST_F(EventTest, Planes) {
  event = Event(3,7);
  EXPECT_EQ(event.plane1(), 3);
  EXPECT_EQ(event.plane2(), 7);
}

TEST_F(EventTest, Insert) {
  event.insert({0, 0, 0, 0});
  EXPECT_EQ(event.c1.hit_count(), 1);
  event.insert({0, 0, 0, 1});
  EXPECT_EQ(event.c2.hit_count(), 1);
}

TEST_F(EventTest, Empty) {
  EXPECT_TRUE(event.empty());
  event.insert({0, 0, 0, 0});
  EXPECT_FALSE(event.empty());
}

TEST_F(EventTest, Clear) {
  event.insert({0, 0, 0, 0});
  EXPECT_FALSE(event.empty());
  event.clear();
  EXPECT_TRUE(event.empty());
}

TEST_F(EventTest, Merge) {
  Cluster x;
  x.insert({0, 0, 0, 0});
  x.insert({0, 0, 0, 0});
  event.merge(x);
  EXPECT_FALSE(event.empty());
  EXPECT_EQ(event.c1.hit_count(), 2);
}

TEST_F(EventTest, MergeTwice) {
  Cluster x;
  x.insert({0, 0, 0, 0});
  x.insert({0, 0, 0, 0});
  event.merge(x);
  EXPECT_EQ(event.c1.hit_count(), 2);

  x.clear();
  x.insert({0, 0, 0, 0});
  x.insert({0, 0, 0, 0});
  x.insert({0, 0, 0, 0});
  event.merge(x);
  EXPECT_EQ(event.c1.hit_count(), 5);
}

TEST_F(EventTest, MergeXY) {
  Cluster x, y;

  x.insert({0, 0, 0, 0});
  x.insert({0, 0, 0, 0});
  event.merge(x);

  y.insert({0, 0, 0, 1});
  y.insert({0, 0, 0, 1});
  y.insert({0, 0, 0, 1});
  event.merge(y);
  EXPECT_EQ(event.c1.hit_count(), 2);
  EXPECT_EQ(event.c2.hit_count(), 3);
}

TEST_F(EventTest, TimeSpanEmpty) {
  EXPECT_EQ(event.time_span(), 0);
}

TEST_F(EventTest, TimeSpanXOnly) {
  Cluster x;

  x.insert({3, 0, 0, 0});
  x.insert({7, 0, 0, 0});
  event.merge(x);

  EXPECT_EQ(event.time_end(), 7);
  EXPECT_EQ(event.time_start(), 3);
  EXPECT_EQ(event.time_span(), 5);
}

TEST_F(EventTest, TimeSpanYOnly) {
  Cluster y;

  y.insert({5, 0, 0, 1});
  y.insert({1, 0, 0, 1});
  event.merge(y);
  EXPECT_EQ(event.time_end(), 5);
  EXPECT_EQ(event.time_start(), 1);
  EXPECT_EQ(event.time_span(), 5);
}

TEST_F(EventTest, TimeSpan) {
  Cluster x, y;

  x.insert({3, 0, 0, 0});
  x.insert({7, 0, 0, 0});
  event.merge(x);

  y.insert({5, 0, 0, 1});
  y.insert({1, 0, 0, 1});
  event.merge(y);
  EXPECT_EQ(event.time_end(), 7);
  EXPECT_EQ(event.time_start(), 1);
  EXPECT_EQ(event.time_span(), 7);
}

TEST_F(EventTest, IgnoreInvalidPlane) {
  event = Event(1, 3);

  event.insert({0, 0, 0, 0});
  event.insert({5, 0, 0, 1});
  event.insert({10, 0, 0, 2});
  event.insert({50, 0, 0, 3});
  EXPECT_EQ(event.time_span(), 46);
}

TEST_F(EventTest, TimeOverlapNoOverlap) {
  Cluster cluster;
  EXPECT_EQ(event.time_overlap(cluster), 0);

  event.insert({0, 0, 0, 0});
  event.insert({5, 0, 0, 0});
  cluster.insert({6, 0, 0, 0});
  cluster.insert({12, 0, 0, 0});
  EXPECT_EQ(event.time_overlap(cluster), 0);
}

TEST_F(EventTest, TimeOverlapInternalPoint) {
  Cluster cluster;
  cluster.insert({3, 0, 0, 0});
  event.insert({0, 0, 0, 0});
  event.insert({6, 0, 0, 0});
  EXPECT_EQ(event.time_overlap(cluster), 1);
}

TEST_F(EventTest, TimeOverlapTouchEdge) {
  Cluster cluster;
  event.insert({0, 0, 0, 0});
  event.insert({6, 0, 0, 0});
  cluster.insert({6, 0, 0, 0});
  cluster.insert({12, 0, 0, 0});
  EXPECT_EQ(event.time_overlap(cluster), 1);
}

TEST_F(EventTest, Overlap) {
  Cluster cluster;

  event.insert({0, 0, 0, 0});
  event.insert({7, 0, 0, 0});
  EXPECT_EQ(event.time_overlap(cluster), 0);

  cluster.insert({12, 0, 0, 0});
  EXPECT_EQ(event.time_overlap(cluster), 0);

  cluster.insert({6, 0, 0, 0});
  EXPECT_EQ(event.time_overlap(cluster), 2);

  cluster.insert({5, 0, 0, 0});
  EXPECT_EQ(event.time_overlap(cluster), 3);
}

TEST_F(EventTest, DebugPrint) {
  event.insert({7, 5, 1, 0});
  event.insert({0, 5, 1, 1});

  MESSAGE() << "NOT A UNIT TEST: please manually check output\n";
  MESSAGE() << "SIMPLE:\n" << event.debug() << "\n";
  MESSAGE() << "VERBOSE:\n" << event.debug(true) << "\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
