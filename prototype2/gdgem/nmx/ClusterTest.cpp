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

TEST_F(ClusterTest, AnalyzeInvalid) {
  auto result = utpcAnalyzer(false, 2, 2).analyze(cluster);
  EXPECT_TRUE(std::isnan(result.utpc_center));
}

TEST_F(ClusterTest, AnalyzeAverage) {
  Hit e;
  e.coordinate = 0;
  e.weight = 2;
  cluster.insert(e);
  auto result = utpcAnalyzer(false, 1, 1).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 0);
  e.coordinate = 1;
  e.weight = 4;
  cluster.insert(e);
  e.coordinate = 2;
  e.weight = 4;
  cluster.insert(e);
  result = utpcAnalyzer(false, 1, 1).analyze(cluster);
  EXPECT_EQ(cluster.hit_count(), 3);
  EXPECT_EQ(result.utpc_center, 1);
  result = utpcAnalyzer(true, 1, 1).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 1.2);
  EXPECT_EQ(result.utpc_center_rounded(), 1);
}

TEST_F(ClusterTest, AnalyzeUncert) {
  e.weight = 1;

  e.time = e.coordinate = 0;
  cluster.insert(e);
  e.time = e.coordinate = 1;
  cluster.insert(e);
  e.time = e.coordinate = 2;
  cluster.insert(e);

  auto result = utpcAnalyzer(true, 1, 1).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 2);
  EXPECT_EQ(result.uncert_lower, 1);
  EXPECT_EQ(result.uncert_upper, 1);

  result = utpcAnalyzer(true, 2, 2).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 2);
  EXPECT_EQ(result.uncert_lower, 1);
  EXPECT_EQ(result.uncert_upper, 2);

  e.coordinate = 31;
  cluster.insert(e);
  result = utpcAnalyzer(true, 2, 2).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 16.5);
  EXPECT_EQ(result.uncert_lower, 30);
  EXPECT_EQ(result.uncert_upper, 31);

  result = utpcAnalyzer(true, 5, 5).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 16.5);
  EXPECT_EQ(result.uncert_lower, 30);
  EXPECT_EQ(result.uncert_upper, 32);

  EXPECT_EQ(result.utpc_center, 16.5);
  EXPECT_EQ(result.utpc_center_rounded(), 17);
}

/// \todo cluster plane identity tests

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
