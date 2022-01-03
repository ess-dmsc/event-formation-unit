/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/reduction/matching/CenterMatcher.h>
#include <common/reduction/matching/EndMatcher.h>
#include <common/testutils/TestBase.h>

class CenterMatcherTest : public TestBase {
protected:

  ClusterContainer x, y;
  CenterMatcher matcher{1000, 0, 1};

  void SetUp() override { 
    matcher.set_max_delta_time(250);
    matcher.set_time_algorithm("center-of-mass");	
  }
  
  void TearDown() override { }
  
   
  Cluster mock_cluster(uint8_t plane, uint64_t time, uint16_t coordinate) {
    Cluster ret;
    Hit e;
    e.plane = plane;
    e.weight = 1;
    e.time = time;
    e.coordinate = coordinate;
    ret.insert(e);
    return ret;
  }
  
};

TEST_F(CenterMatcherTest, Constructor) {
  ASSERT_EQ(matcher.stats_event_count, 0);
  ASSERT_EQ(matcher.matched_events.size(), 0);
  matcher.match(true); // ever called with false?
}

TEST_F(CenterMatcherTest, PrintConfig) {
  MESSAGE() << "NOT A UNIT TEST: please manually check output\n";
  MESSAGE() << "CONFIG:\n" << matcher.config("  ");
}

TEST_F(CenterMatcherTest, X) {
  x.push_back(mock_cluster(0, 100, 10));
  matcher.insert(0, x);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 1);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 0);
}

TEST_F(CenterMatcherTest, Y) {
  y.push_back(mock_cluster(1, 100, 100));
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 1);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 1);
}

TEST_F(CenterMatcherTest, X_X_SmallDeltaT) {
  x.push_back(mock_cluster(0, 100, 10));
  x.push_back(mock_cluster(0, 120, 20));
  matcher.insert(0, x);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 0);
}

TEST_F(CenterMatcherTest, X_X_LargeDeltaT) {
  x.push_back(mock_cluster(0, 100, 10));
  x.push_back(mock_cluster(0, 1000, 20));
  matcher.insert(0, x);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 0);
}

TEST_F(CenterMatcherTest, Y_Y_SmallDeltaT) {
  y.push_back(mock_cluster(1, 100, 10));
  y.push_back(mock_cluster(1, 120, 20));
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 1);
}

TEST_F(CenterMatcherTest, Y_Y_LargeDeltaT) {
  y.push_back(mock_cluster(1, 100, 10));
  y.push_back(mock_cluster(1, 1000, 20));
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 1);
}

TEST_F(CenterMatcherTest, X_Y_SmallDeltaT) {
  x.push_back(mock_cluster(0, 100, 10));
  y.push_back(mock_cluster(1, 125, 100));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 1);
  EXPECT_EQ(matcher.matched_events.size(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 1);
}

TEST_F(CenterMatcherTest, X_Y_LargeDeltaT) {
  x.push_back(mock_cluster(0, 100, 10));
  y.push_back(mock_cluster(1, 1000, 100));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 1);
}

TEST_F(CenterMatcherTest, X_Y_X) {
  x.push_back(mock_cluster(0, 100, 10));
  y.push_back(mock_cluster(1, 150, 20));
  x.push_back(mock_cluster(0, 160, 200));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.coord_center(), 10);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.coord_center(), 20);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.coord_center(), 200);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.time_center(), 100);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.time_center(), 150);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.time_center(), 160);
}

TEST_F(CenterMatcherTest, X_X_Y) {
  x.push_back(mock_cluster(0, 100, 10));
  x.push_back(mock_cluster(0, 150, 20));
  y.push_back(mock_cluster(1, 160, 200));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.coord_center(), 10);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.coord_center(), 20);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.coord_center(), 200);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.time_center(), 100);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.time_center(), 150);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.time_center(), 160);
}

TEST_F(CenterMatcherTest, X_X_Y_Y) {
  x.push_back(mock_cluster(0, 100, 10));
  y.push_back(mock_cluster(0, 140, 100));
  x.push_back(mock_cluster(1, 150, 20));
  y.push_back(mock_cluster(1, 200, 200));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 3);
  EXPECT_EQ(matcher.matched_events.size(), 3);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.coord_center(), 10);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.coord_center(), 200);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.time_center(), 100);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.time_center(), 200);
}


TEST_F(CenterMatcherTest, Y_Y_X_X) {
  x.push_back(mock_cluster(1, 100, 10));
  y.push_back(mock_cluster(1, 140, 100));
  x.push_back(mock_cluster(0, 150, 20));
  y.push_back(mock_cluster(0, 200, 200));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 3);
  EXPECT_EQ(matcher.matched_events.size(), 3);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 0);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.coord_center(), 10);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.coord_center(), 200);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.time_center(), 100);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.time_center(), 200);
}


TEST_F(CenterMatcherTest, X_Y_X_Y) {
  x.push_back(mock_cluster(0, 100, 10));
  y.push_back(mock_cluster(1, 140, 100));
  x.push_back(mock_cluster(0, 150, 20));
  y.push_back(mock_cluster(1, 200, 200));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.coord_center(), 10);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.coord_center(), 100);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.coord_center(), 20);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.coord_center(), 200);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.time_center(), 100);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.time_center(), 140);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.time_center(), 150);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.time_center(), 200);
}

TEST_F(CenterMatcherTest, X_Y_Y_X) {
  x.push_back(mock_cluster(0, 100, 10));
  y.push_back(mock_cluster(1, 140, 100));
  y.push_back(mock_cluster(1, 140, 200));
  x.push_back(mock_cluster(0, 200, 20));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.coord_center(), 10);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.coord_center(), 100);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.coord_center(), 20);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.coord_center(), 200);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.time_center(), 100);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.time_center(), 140);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.time_center(), 200);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.time_center(), 140);
}

TEST_F(CenterMatcherTest, Y_X_X_Y) {
  y.push_back(mock_cluster(1, 100, 10));
  x.push_back(mock_cluster(0, 140, 100));
  x.push_back(mock_cluster(0, 140, 200));
  y.push_back(mock_cluster(1, 200, 20));
  matcher.insert(0, x);
  matcher.insert(1, y);
  matcher.match(true);
  EXPECT_EQ(matcher.stats_event_count, 2);
  EXPECT_EQ(matcher.matched_events.size(), 2);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.hit_count(), 1);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.coord_center(), 100);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.coord_center(), 10);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.coord_center(), 200);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.coord_center(), 20);
  EXPECT_EQ(matcher.matched_events.front().ClusterA.time_center(), 140);
  EXPECT_EQ(matcher.matched_events.front().ClusterB.time_center(), 100);
  EXPECT_EQ(matcher.matched_events.back().ClusterA.time_center(), 140);
  EXPECT_EQ(matcher.matched_events.back().ClusterB.time_center(), 200);
}

 
  
// \todo do more tests

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

