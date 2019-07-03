/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/clustering/EndMatcher.h>
#include <test/TestBase.h>

class EndMatcherTest : public TestBase {
protected:
  ClusterContainer x, y;
  EndMatcher matcher{600, 0, 1, 255};

  void SetUp() override {
    matcher.set_max_delta_time(200);
  }

  Cluster mock_cluster(uint8_t plane, uint16_t strip_start, uint16_t strip_end,
                       uint64_t time_start, uint64_t time_end) {
    Cluster ret;
    Hit e;
    e.plane = plane;
    e.weight = 1;
    double time_step = (time_end - time_start) / 10.0;
    for (e.time = time_start; e.time <= time_end; e.time += time_step)
      for (e.coordinate = strip_start; e.coordinate <= strip_end; ++e.coordinate)
        ret.insert(e);
    e.time = time_end;
    ret.insert(e);
    return ret;
  }

};

TEST_F(EndMatcherTest, OneX) {
  x.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher.insert(0, x);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 1);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 122);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 0);
}

TEST_F(EndMatcherTest, OneY) {
  y.push_back(mock_cluster(1, 0, 10, 0, 200));
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 1);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 122);
}

TEST_F(EndMatcherTest, TwoX) {
  x.push_back(mock_cluster(0, 0, 10, 0, 200));
  x.push_back(mock_cluster(0, 0, 10, 500, 700));
  matcher.insert(0, x);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.back().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 122);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().cluster1.hit_count(), 122);
  EXPECT_EQ(matcher.matched_events.back().cluster2.hit_count(), 0);
}

TEST_F(EndMatcherTest, TwoY) {
  y.push_back(mock_cluster(1, 0, 10, 0, 200));
  y.push_back(mock_cluster(1, 0, 10, 500, 700));
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.back().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 122);
  EXPECT_EQ(matcher.matched_events.back().cluster1.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().cluster2.hit_count(), 122);
}

TEST_F(EndMatcherTest, OneXOneY) {
  x.push_back(mock_cluster(0, 0, 10, 0, 200));
  y.push_back(mock_cluster(1, 0, 10, 500, 700));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.back().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 122);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().cluster1.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().cluster2.hit_count(), 122);
}

TEST_F(EndMatcherTest, OneXY) {
  x.push_back(mock_cluster(0, 0, 10, 0, 200));
  y.push_back(mock_cluster(1, 0, 10, 0, 200));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 1);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 201);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 122);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 122);
}

TEST_F(EndMatcherTest, TwoXY) {
  x.push_back(mock_cluster(0, 0, 10, 0, 200));
  y.push_back(mock_cluster(1, 0, 10, 1, 300));
  x.push_back(mock_cluster(0, 0, 10, 600, 800));
  y.push_back(mock_cluster(1, 0, 10, 650, 850));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().time_span(), 301);
  EXPECT_EQ(matcher.matched_events.back().time_span(), 251);
  EXPECT_EQ(matcher.matched_events.front().cluster1.hit_count(), 122);
  EXPECT_EQ(matcher.matched_events.front().cluster2.hit_count(), 122);
  EXPECT_EQ(matcher.matched_events.back().cluster1.hit_count(), 122);
  EXPECT_EQ(matcher.matched_events.back().cluster2.hit_count(), 122);
}

TEST_F(EndMatcherTest, JustIntside) {
  x.push_back(mock_cluster(0, 0, 10, 0, 200));
  y.push_back(mock_cluster(1, 0, 10, 200, 400));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.matched_events.size(), 1);
}

TEST_F(EndMatcherTest, JustOutside) {
  x.push_back(mock_cluster(0, 0, 10, 0, 200));
  y.push_back(mock_cluster(1, 0, 10, 200, 401));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.matched_events.size(), 2);
}

TEST_F(EndMatcherTest, DontForce) {
  x.push_back(mock_cluster(0, 0, 10, 0, 200));
  y.push_back(mock_cluster(1, 0, 10, 200, 401));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(false);
  EXPECT_EQ(matcher.matched_events.size(), 0);

  x.push_back(mock_cluster(0, 0, 10, 800, 1000));
  matcher.insert(0, x);
  matcher.match(false);
  EXPECT_EQ(matcher.matched_events.size(), 0);

  y.push_back(mock_cluster(1, 0, 10, 900, 1000));
  matcher.insert(1, y);
  matcher.match(false);
  EXPECT_EQ(matcher.matched_events.size(), 0);

  x.push_back(mock_cluster(0, 0, 10, 1000, 1200));
  matcher.insert(0, x);
  matcher.match(false);
  EXPECT_EQ(matcher.matched_events.size(), 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
