/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Cluster.h>
#include <cmath>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

using namespace Gem;

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
  EXPECT_EQ(cluster.hits.size(), 1);
  e.strip = 2;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.hits.size(), 2);
  e.strip = 3;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.hits.size(), 3);
}


TEST_F(ClusterTest, AdcSum) {
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.adc_sum, 0);
  e.adc = 2;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.adc_sum, 2);
  e.adc = 40;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.adc_sum, 42);
}

TEST_F(ClusterTest, TimeSpan) {
  EXPECT_EQ(cluster.time_span(), 0);

  e.time = 10;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_start, 10);
  EXPECT_EQ(cluster.time_end, 10);
  EXPECT_EQ(cluster.time_span(), 0);

  e.time = 20;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_start, 10);
  EXPECT_EQ(cluster.time_end, 20);
  EXPECT_EQ(cluster.time_span(), 10);

  e.time = 5;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_start, 5);
  EXPECT_EQ(cluster.time_end, 20);
  EXPECT_EQ(cluster.time_span(), 15);
}

TEST_F(ClusterTest, StripSpan) {
  EXPECT_EQ(cluster.strip_span(), 0);

  e.strip = 0;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.strip_start, 0);
  EXPECT_EQ(cluster.strip_end, 0);
  EXPECT_EQ(cluster.strip_span(), 1);

  e.strip = 10;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.strip_start, 0);
  EXPECT_EQ(cluster.strip_end, 10);
  EXPECT_EQ(cluster.strip_span(), 11);

  e.strip = 41;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.strip_start, 0);
  EXPECT_EQ(cluster.strip_end, 41);
  EXPECT_EQ(cluster.strip_span(), 42);
}

// 0-adc-valued hits will be a problem
TEST_F(ClusterTest, TimeMass) {
  EXPECT_EQ(cluster.time_mass, 0);
  EXPECT_TRUE(std::isnan(cluster.time_center()));

  e.adc = 2;
  e.time = 10;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_mass, 20);
  EXPECT_EQ(cluster.time_center(), 10);

  e.adc = 8;
  e.time = 0;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_mass, 20);
  EXPECT_EQ(cluster.time_center(), 2);
}

TEST_F(ClusterTest, TimeStrips) {
  EXPECT_EQ(cluster.strip_mass, 0);
  EXPECT_TRUE(std::isnan(cluster.strip_center()));

  e.adc = 2;
  e.strip = 10;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.strip_mass, 20);
  EXPECT_EQ(cluster.strip_center(), 10);

  e.adc = 8;
  e.strip = 0;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.strip_mass, 20);
  EXPECT_EQ(cluster.strip_center(), 2);
}

TEST_F(ClusterTest, TimeNoOverlap) {
  Cluster cluster2;
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);

  // this is degenerate case
  e.time = 6;
  cluster.insert_hit(e);
  e.time = -6;
  cluster.insert_hit(e);
  EXPECT_EQ(cluster.time_overlap(cluster2), 0);
  EXPECT_TRUE(cluster.time_touch(cluster2));

  // these are adjacent
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

  EXPECT_EQ(cluster.hits.size(), 3);
  EXPECT_EQ(cluster2.hits.size(), 0);
}

TEST_F(ClusterTest, MergeToEmpty) {
  Cluster cluster2;
  cluster2.insert_hit(e);
  cluster2.insert_hit(e);
  cluster2.insert_hit(e);

  /// \todo old cluster stats should be reset
  cluster.merge(cluster2);

  EXPECT_EQ(cluster.hits.size(), 3);
  EXPECT_EQ(cluster2.hits.size(), 0);
}


TEST_F(ClusterTest, Merge) {
  e.adc = 1;

  e.strip = 5;
  e.time = 0;
  cluster.insert_hit(e);
  e.time = 7;
  cluster.insert_hit(e);

  Cluster cluster2;
  e.strip = 15;
  e.time = 12;
  cluster2.insert_hit(e);
  e.time = 6;
  cluster2.insert_hit(e);
  e.time = 5;
  cluster2.insert_hit(e);

  /// \todo old cluster stats should be reset
  cluster.merge(cluster2);

  EXPECT_EQ(cluster.hits.size(), 5);
  EXPECT_EQ(cluster.time_span(), 12);
  EXPECT_EQ(cluster.strip_span(), 11);
  EXPECT_EQ(cluster.adc_sum, 5);
}


TEST_F(ClusterTest, AnalyzeInvalid) {
  EXPECT_TRUE(std::isnan(cluster.utpc_center));
  cluster.analyze(false, 2, 2);
  EXPECT_TRUE(std::isnan(cluster.utpc_center));
}

TEST_F(ClusterTest, AnalyzeAverage) {
  Hit e;
  e.strip = 0;
  e.adc = 2;
  cluster.insert_hit(e);
  cluster.analyze(false, 1, 1);
  EXPECT_EQ(cluster.utpc_center, 0);
  e.strip = 1;
  e.adc = 4;
  cluster.insert_hit(e);
  e.strip = 2;
  e.adc = 4;
  cluster.insert_hit(e);
  cluster.analyze(false, 1, 1);
  EXPECT_EQ(cluster.hits.size(), 3);
  EXPECT_EQ(cluster.utpc_center, 1);
  cluster.analyze(true, 1, 1);
  EXPECT_EQ(cluster.utpc_center, 1.2);
  EXPECT_EQ(cluster.utpc_center_rounded(), 1);
}

TEST_F(ClusterTest, AnalyzeUncert) {
  e.adc = 1;

  e.time = e.strip = 0;
  cluster.insert_hit(e);
  e.time = e.strip = 1;
  cluster.insert_hit(e);
  e.time = e.strip = 2;
  cluster.insert_hit(e);

  cluster.analyze(true, 1, 1);
  EXPECT_EQ(cluster.utpc_center, 2);
  EXPECT_EQ(cluster.uncert_lower, 1);
  EXPECT_EQ(cluster.uncert_upper, 1);

  cluster.analyze(true, 2, 2);
  EXPECT_EQ(cluster.utpc_center, 2);
  EXPECT_EQ(cluster.uncert_lower, 1);
  EXPECT_EQ(cluster.uncert_upper, 2);

  e.strip = 31;
  cluster.insert_hit(e);
  cluster.analyze(true, 2, 2);
  EXPECT_EQ(cluster.utpc_center, 16.5);
  EXPECT_EQ(cluster.uncert_lower, 30);
  EXPECT_EQ(cluster.uncert_upper, 31);

  cluster.analyze(true, 5, 5);
  EXPECT_EQ(cluster.utpc_center, 16.5);
  EXPECT_EQ(cluster.uncert_lower, 30);
  EXPECT_EQ(cluster.uncert_upper, 32);

  EXPECT_EQ(cluster.utpc_center, 16.5);
  EXPECT_EQ(cluster.utpc_center_rounded(), 17);
}

/// \todo cluster plane identity tests

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
