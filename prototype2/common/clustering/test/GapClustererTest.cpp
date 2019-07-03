/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/clustering/GapClusterer.h>

#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <test/TestBase.h>
#include <functional>

class GapClustererTest : public TestBase {
protected:
  void mock_cluster(HitVector &ret, uint16_t strip_start, uint16_t strip_end, uint16_t strip_step,
                    uint64_t time_start, uint64_t time_end, uint64_t time_step) {
    Hit e;
    e.plane = 0;
    e.weight = 1;
    for (e.time = time_start; e.time <= time_end; e.time += time_step)
      for (e.coordinate = strip_start; e.coordinate <= strip_end; e.coordinate += strip_step)
        ret.push_back(e);
  }
};

TEST_F(GapClustererTest, ZeroTimeGap) {
  HitVector hc;
  mock_cluster(hc, 0, 0, 1, 1, 10, 1);

  GapClusterer gc(0, 0);
  gc.cluster(hc);

  EXPECT_EQ(gc.stats_cluster_count, 9);
  EXPECT_EQ(gc.clusters.size(), 9);

  gc.flush();
  EXPECT_EQ(gc.stats_cluster_count, 10);
  EXPECT_EQ(gc.clusters.size(), 10);
}

TEST_F(GapClustererTest, JustUnderFiveTimeGap) {
  HitVector hc;
  mock_cluster(hc, 0, 0, 1, 0, 40, 4);

  GapClusterer gc(5, 0);
  gc.cluster(hc);

  EXPECT_EQ(gc.stats_cluster_count, 0);
  EXPECT_EQ(gc.clusters.size(), 0);

  gc.flush();
  EXPECT_EQ(gc.stats_cluster_count, 1);
  EXPECT_EQ(gc.clusters.size(), 1);
}

TEST_F(GapClustererTest, ExactlyFiveTimeGap) {
  HitVector hc;
  mock_cluster(hc, 0, 0, 1, 0, 50, 5);

  GapClusterer gc(5, 0);
  gc.cluster(hc);

  EXPECT_EQ(gc.stats_cluster_count, 0);
  EXPECT_EQ(gc.clusters.size(), 0);

  gc.flush();
  EXPECT_EQ(gc.stats_cluster_count, 1);
  EXPECT_EQ(gc.clusters.size(), 1);
}

TEST_F(GapClustererTest, JustOverFiveTimeGap) {
  HitVector hc;
  mock_cluster(hc, 0, 0, 1, 1, 60, 6);

  GapClusterer gc(5, 0);
  gc.cluster(hc);

  EXPECT_EQ(gc.stats_cluster_count, 9);
  EXPECT_EQ(gc.clusters.size(), 9);

  gc.flush();
  EXPECT_EQ(gc.stats_cluster_count, 10);
  EXPECT_EQ(gc.clusters.size(), 10);
}

TEST_F(GapClustererTest, ZeroCoordGap) {
  HitVector hc;
  mock_cluster(hc, 1, 10, 1, 1, 10, 1);

  GapClusterer gc(0, 0);
  gc.cluster(hc);

  EXPECT_EQ(gc.stats_cluster_count, 90);
  EXPECT_EQ(gc.clusters.size(), 90);

  gc.flush();
  EXPECT_EQ(gc.stats_cluster_count, 100);
  EXPECT_EQ(gc.clusters.size(), 100);
}

TEST_F(GapClustererTest, JustUnderFiveCoordGap) {
  HitVector hc;
  mock_cluster(hc, 1, 40, 4, 1, 10, 1);

  GapClusterer gc(0, 5);
  gc.cluster(hc);

  EXPECT_EQ(gc.stats_cluster_count, 9);
  EXPECT_EQ(gc.clusters.size(), 9);

  gc.flush();
  EXPECT_EQ(gc.stats_cluster_count, 10);
  EXPECT_EQ(gc.clusters.size(), 10);
}

TEST_F(GapClustererTest, ExactlyFiveCoordGap) {
  HitVector hc;
  mock_cluster(hc, 1, 50, 5, 1, 10, 1);

  GapClusterer gc(0, 5);
  gc.cluster(hc);

  EXPECT_EQ(gc.stats_cluster_count, 9);
  EXPECT_EQ(gc.clusters.size(), 9);

  gc.flush();
  EXPECT_EQ(gc.stats_cluster_count, 10);
  EXPECT_EQ(gc.clusters.size(), 10);
}

TEST_F(GapClustererTest, JustOverFiveCoordGap) {
  HitVector hc;
  mock_cluster(hc, 1, 60, 6, 1, 10, 1);

  GapClusterer gc(0, 5);
  gc.cluster(hc);

  EXPECT_EQ(gc.stats_cluster_count, 90);
  EXPECT_EQ(gc.clusters.size(), 90);

  gc.flush();
  EXPECT_EQ(gc.stats_cluster_count, 100);
  EXPECT_EQ(gc.clusters.size(), 100);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
