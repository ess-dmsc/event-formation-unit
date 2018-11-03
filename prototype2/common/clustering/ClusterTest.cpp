/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/clustering/Cluster.h>
#include <cmath>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class ClusterTest : public TestBase {
protected:
  Hit e;
  Cluster cluster;
  virtual void SetUp() { }
  virtual void TearDown() { }
};

TEST_F(ClusterTest, Insert) {
  Hit e;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.hit_count(), 1);
  e.coordinate = 2;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.hit_count(), 2);
  e.coordinate = 3;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.hit_count(), 3);
}


TEST_F(ClusterTest, AdcSum) {
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.weight_sum(), 0);
  e.weight = 2;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.weight_sum(), 2);
  e.weight = 40;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.weight_sum(), 42);
}

TEST_F(ClusterTest, TimeSpan) {
  EXPECT_EQ(cluster.time_span(), 0);

  e.time = 10;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_start(), 10);
  EXPECT_EQ(cluster.time_end(), 10);
  EXPECT_EQ(cluster.time_span(), 0);

  e.time = 20;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_start(), 10);
  EXPECT_EQ(cluster.time_end(), 20);
  EXPECT_EQ(cluster.time_span(), 10);

  e.time = 5;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_start(), 5);
  EXPECT_EQ(cluster.time_end(), 20);
  EXPECT_EQ(cluster.time_span(), 15);
}

TEST_F(ClusterTest, StripSpan) {
  EXPECT_EQ(cluster.coord_span(), 0);

  e.coordinate = 0;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.coord_start(), 0);
  EXPECT_EQ(cluster.coord_end(), 0);
  EXPECT_EQ(cluster.coord_span(), 1);

  e.coordinate = 10;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.coord_start(), 0);
  EXPECT_EQ(cluster.coord_end(), 10);
  EXPECT_EQ(cluster.coord_span(), 11);

  e.coordinate = 41;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.coord_start(), 0);
  EXPECT_EQ(cluster.coord_end(), 41);
  EXPECT_EQ(cluster.coord_span(), 42);
}

// 0-weight-valued hits will be a problem
TEST_F(ClusterTest, TimeMass) {
  EXPECT_EQ(cluster.time_mass(), 0);
  EXPECT_TRUE(std::isnan(cluster.time_center()));

  e.weight = 2;
  e.time = 10;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_mass(), 20);
  EXPECT_EQ(cluster.time_center(), 10);

  e.weight = 8;
  e.time = 0;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_mass(), 20);
  EXPECT_EQ(cluster.time_center(), 2);
}

TEST_F(ClusterTest, TimeStrips) {
  EXPECT_EQ(cluster.coord_mass(), 0);
  EXPECT_TRUE(std::isnan(cluster.coord_center()));

  e.weight = 2;
  e.coordinate = 10;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.coord_mass(), 20);
  EXPECT_EQ(cluster.coord_center(), 10);

  e.weight = 8;
  e.coordinate = 0;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.coord_mass(), 20);
  EXPECT_EQ(cluster.coord_center(), 2);
}

TEST_F(ClusterTest, TimeNoOverlap) {
  Cluster cluster2;
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);

  // this is degenerate case
  e.time = 3;
  cluster2.insert_hit(e);
  e.time = 6;
  cluster.insert_hit(e);
  e.time = 0;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);
  EXPECT_TRUE(cluster.time_touch(cluster2));

  // these are adjacent
  cluster2 = Cluster();
  e.time = 6;
  cluster2.insert_hit(e);
  e.time = 12;
  cluster2.insert_hit(e);
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);
  EXPECT_TRUE(cluster.time_touch(cluster2));
}

TEST_F(ClusterTest, Overlap) {
  Cluster cluster2;

  e.time = 0;
  cluster.insert_hit(e);
  e.time = 7;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);

  e.time = 12;
  cluster2.insert_hit(e);
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);

  e.time = 6;
  cluster2.insert_hit(e);
  EXPECT_EQ(cluster.time_overlap(cluster2), 1);

  e.time = 5;
  cluster2.insert_hit(e);
  EXPECT_EQ(cluster.time_overlap(cluster2), 2);
}

TEST_F(ClusterTest, MergeEmpty) {
  cluster.insert_hit(e);
  cluster.insert_hit(e);
  cluster.insert_hit(e);

  Cluster cluster2;
  cluster.merge(cluster2);

  EXPECT_EQ(cluster.hit_count(), 3);
  EXPECT_EQ(cluster2.hit_count(), 0);
}

TEST_F(ClusterTest, MergeToEmpty) {
  Cluster cluster2;
  cluster2.insert_hit(e);
  cluster2.insert_hit(e);
  cluster2.insert_hit(e);

  /// \todo old cluster stats should be reset
  cluster.merge(cluster2);

  EXPECT_EQ(cluster.hit_count(), 3);
  EXPECT_EQ(cluster2.hit_count(), 0);
}


TEST_F(ClusterTest, Merge) {
  e.weight = 1;

  e.coordinate = 5;
  e.time = 0;
  cluster.insert_hit(e);
  e.time = 7;
  cluster.insert_hit(e);

  Cluster cluster2;
  e.coordinate = 15;
  e.time = 12;
  cluster2.insert_hit(e);
  e.time = 6;
  cluster2.insert_hit(e);
  e.time = 5;
  cluster2.insert_hit(e);

  /// \todo old cluster stats should be reset
  cluster.merge(cluster2);

  EXPECT_EQ(cluster.hit_count(), 5);
  EXPECT_EQ(cluster.time_span(), 12);
  EXPECT_EQ(cluster.coord_span(), 11);
  EXPECT_EQ(cluster.weight_sum(), 5);
}

/// \todo cluster plane identity tests

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
