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
  UtpcCluster cluster;
  virtual void SetUp() { }
  virtual void TearDown() { }
};

TEST_F(ClusterTest, AnalyzeInvalid) {
  EXPECT_TRUE(std::isnan(cluster.utpc_center));
  cluster.analyze(false, 2, 2);
  EXPECT_TRUE(std::isnan(cluster.utpc_center));
}

TEST_F(ClusterTest, AnalyzeAverage) {
  Hit e;
  e.coordinate = 0;
  e.weight = 2;
  cluster.insert(e);
  cluster.analyze(false, 1, 1);
  EXPECT_EQ(cluster.utpc_center, 0);
  e.coordinate = 1;
  e.weight = 4;
  cluster.insert(e);
  e.coordinate = 2;
  e.weight = 4;
  cluster.insert(e);
  cluster.analyze(false, 1, 1);
  EXPECT_EQ(cluster.hit_count(), 3);
  EXPECT_EQ(cluster.utpc_center, 1);
  cluster.analyze(true, 1, 1);
  EXPECT_EQ(cluster.utpc_center, 1.2);
  EXPECT_EQ(cluster.utpc_center_rounded(), 1);
}

TEST_F(ClusterTest, AnalyzeUncert) {
  e.weight = 1;

  e.time = e.coordinate = 0;
  cluster.insert(e);
  e.time = e.coordinate = 1;
  cluster.insert(e);
  e.time = e.coordinate = 2;
  cluster.insert(e);

  cluster.analyze(true, 1, 1);
  EXPECT_EQ(cluster.utpc_center, 2);
  EXPECT_EQ(cluster.uncert_lower, 1);
  EXPECT_EQ(cluster.uncert_upper, 1);

  cluster.analyze(true, 2, 2);
  EXPECT_EQ(cluster.utpc_center, 2);
  EXPECT_EQ(cluster.uncert_lower, 1);
  EXPECT_EQ(cluster.uncert_upper, 2);

  e.coordinate = 31;
  cluster.insert(e);
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
