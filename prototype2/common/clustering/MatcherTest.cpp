/** Copyright (C) 2017 European Spallation Source ERIC */

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <common/clustering/Matcher.h>
#include <test/TestBase.h>
#include <functional>


#define UNUSED __attribute__((unused))

class ClusterMatcherTest : public TestBase {
protected:
  //Maximum cluster time difference between matching clusters in x and y
  //Cluster time is either calculated with center-of-mass or uTPC method
  double pDeltaTimePlanes = 200;
  // Maximum time span for total cluster (x or y)
  // double pDeltaTimeSpan = 500;

  std::shared_ptr<Matcher> matcher;

  virtual void SetUp() {
    matcher = std::make_shared<Matcher>(pDeltaTimePlanes);
  }

  virtual void TearDown() {
  }

  Cluster mock_cluster(uint8_t plane, uint16_t strip_start, uint16_t strip_end,
                       double time_start, double time_end) {
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

TEST_F(ClusterMatcherTest, MergeNoClusters) {
  ClusterContainer c;
  MESSAGE() << "This it NOT a test, as novalidation occurs (cant access private fields)\n";
  matcher->insert(0, c);
}

TEST_F(ClusterMatcherTest, OneX) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->match(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 201);
  EXPECT_EQ(matcher->matched_clusters.front().c1.hit_count(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().c2.hit_count(), 0);
}

TEST_F(ClusterMatcherTest, OneY) {
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0, 10, 0, 200));
  matcher->match(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 201);
  EXPECT_EQ(matcher->matched_clusters.front().c1.hit_count(), 0);
  EXPECT_EQ(matcher->matched_clusters.front().c2.hit_count(), 122);
}

TEST_F(ClusterMatcherTest, TwoX) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0, 10, 500, 700));
  matcher->match(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 201);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 201);
  EXPECT_EQ(matcher->matched_clusters.front().c1.hit_count(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().c2.hit_count(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().c1.hit_count(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().c2.hit_count(), 0);
}

TEST_F(ClusterMatcherTest, TwoY) {
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 500, 700));
  matcher->match(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 201);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 201);
  EXPECT_EQ(matcher->matched_clusters.front().c1.hit_count(), 0);
  EXPECT_EQ(matcher->matched_clusters.front().c2.hit_count(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().c1.hit_count(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().c2.hit_count(), 122);
}

TEST_F(ClusterMatcherTest, OneXOneY) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 500, 700));
  matcher->match(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 201);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 201);
  EXPECT_EQ(matcher->matched_clusters.front().c1.hit_count(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().c2.hit_count(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().c1.hit_count(), 0);
  EXPECT_EQ(matcher->matched_clusters.back().c2.hit_count(), 122);
}

TEST_F(ClusterMatcherTest, OneXY) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 0, 200));
  matcher->match(true);
  ASSERT_EQ(matcher->stats_cluster_count, 1);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 201);
  EXPECT_EQ(matcher->matched_clusters.front().c1.hit_count(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().c2.hit_count(), 122);
}

TEST_F(ClusterMatcherTest, TwoXY) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 1, 300));
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 600, 800));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 650, 850));
  matcher->match(true);
  ASSERT_EQ(matcher->stats_cluster_count, 2);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
  EXPECT_EQ(matcher->matched_clusters.front().time_span(), 301);
  EXPECT_EQ(matcher->matched_clusters.back().time_span(), 251);
  EXPECT_EQ(matcher->matched_clusters.front().c1.hit_count(), 122);
  EXPECT_EQ(matcher->matched_clusters.front().c2.hit_count(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().c1.hit_count(), 122);
  EXPECT_EQ(matcher->matched_clusters.back().c2.hit_count(), 122);
}

TEST_F(ClusterMatcherTest, JustIntside) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 200, 400));
  matcher->match(true);
  ASSERT_EQ(matcher->matched_clusters.size(), 1);
}

TEST_F(ClusterMatcherTest, JustOutside) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 200, 401));
  matcher->match(true);
  ASSERT_EQ(matcher->matched_clusters.size(), 2);
}

TEST_F(ClusterMatcherTest, DontForce) {
  matcher->unmatched_clusters.push_back(mock_cluster(0, 0,10, 0, 200));
  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 200, 401));
  matcher->match(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);

  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 800, 1000));
  matcher->match(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);

  matcher->unmatched_clusters.push_back(mock_cluster(1, 0,10, 900, 1000));
  matcher->match(false);
  ASSERT_EQ(matcher->matched_clusters.size(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
