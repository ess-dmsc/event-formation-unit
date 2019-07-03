/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/clustering/OverlapMatcher.h>
#include <test/TestBase.h>

class OverlapMatcherTest : public TestBase {
protected:
  ClusterContainer x, y;
  OverlapMatcher matcher{600, 0, 1, 255};

  void add_cluster(ClusterContainer &ret, uint8_t plane,
                   uint16_t coord_start, uint16_t coord_end, uint16_t coord_step,
                   uint64_t time_start, uint64_t time_end, uint64_t time_step) {
    Cluster c;
    Hit e;
    e.plane = plane;
    e.weight = 1;
    for (e.time = time_start; e.time <= time_end; e.time += time_step)
      for (e.coordinate = coord_start; e.coordinate <= coord_end; e.coordinate += coord_step)
        c.insert(e);
    ret.push_back(c);
  }
};

TEST_F(OverlapMatcherTest, OneX) {
  add_cluster(x, 0, 1, 10, 1, 0, 200, 20);
  matcher.insert(0, x);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 1);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 110);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 0);
}

TEST_F(OverlapMatcherTest, OneY) {
  add_cluster(y, 1, 1, 10, 1, 0, 200, 20);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 1);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 110);
}

TEST_F(OverlapMatcherTest, TwoX) {
  add_cluster(x, 0, 1, 10, 1, 0, 200, 20);
  add_cluster(x, 0, 1, 10, 1, 500, 700, 20);
  matcher.insert(0, x);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.back().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 110);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().cluster1.hit_count(), 110);
  EXPECT_EQ(matcher.matched_events.back().cluster2.hit_count(), 0);
}

TEST_F(OverlapMatcherTest, TwoY) {
  add_cluster(y, 1, 1, 10, 1, 0, 200, 20);
  add_cluster(y, 1, 1, 10, 1, 500, 700, 20);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.back().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 110);
  EXPECT_EQ(matcher.matched_events.back().cluster1.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().cluster2.hit_count(), 110);
}

TEST_F(OverlapMatcherTest, OneXOneY) {
  add_cluster(x, 0, 1, 10, 1, 0, 200, 20);
  add_cluster(y, 1, 1, 10, 1, 500, 700, 20);
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.back().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 110);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().cluster1.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().cluster2.hit_count(), 110);
}

TEST_F(OverlapMatcherTest, OneXY) {
  add_cluster(x, 0, 1, 10, 1, 0, 200, 20);
  add_cluster(y, 1, 1, 10, 1, 0, 200, 20);
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 1);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 110);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 110);
}

TEST_F(OverlapMatcherTest, TwoXY) {
  add_cluster(x, 0, 1, 10, 1, 0, 200, 1);
  add_cluster(y, 1, 1, 10, 1, 1, 300, 1);
  add_cluster(x, 0, 1, 10, 1, 600, 800, 1);
  add_cluster(y, 1, 1, 10, 1, 650, 850, 1);
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 301);
  EXPECT_EQ(matcher.matched_events.back().time_span(), 251);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 2010);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 3000);
  EXPECT_EQ(matcher.matched_events.back().cluster1.hit_count(), 2010);
  EXPECT_EQ(matcher.matched_events.back().cluster2.hit_count(), 2010);
}

TEST_F(OverlapMatcherTest, JustIntside) {
  add_cluster(x, 0, 0, 10, 1, 0, 200, 1);
  add_cluster(y, 1, 0, 10, 1, 200, 400, 1);
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.matched_events.size(), 1);
}

TEST_F(OverlapMatcherTest, JustOutside) {
  add_cluster(x, 0, 0, 10, 1, 0, 199, 1);
  add_cluster(y, 1, 0, 10, 1, 200, 401, 1);
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.matched_events.size(), 2);
}

TEST_F(OverlapMatcherTest, DontForce) {
  add_cluster(x, 0, 1, 10, 1, 0, 200, 1);
  add_cluster(y, 1, 1, 10, 1, 200, 401, 1);
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(false);
  EXPECT_EQ(matcher.matched_events.size(), 0);

  add_cluster(x, 0, 0, 10, 1, 800, 1000, 1);
  matcher.insert(0, x);
  matcher.match(false);
  EXPECT_EQ(matcher.matched_events.size(), 0);

  add_cluster(y, 1, 0, 10, 1, 900, 1000, 1);
  matcher.insert(1, y);
  matcher.match(false);
  EXPECT_EQ(matcher.matched_events.size(), 0);

  add_cluster(x, 0, 0, 10, 1, 1000, 1200, 1);
  matcher.insert(0, x);
  matcher.match(false);
  EXPECT_EQ(matcher.matched_events.size(), 0);

  // \todo improve this
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
