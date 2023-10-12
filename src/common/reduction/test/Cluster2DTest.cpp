// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief unit tests for Cluster2D.cpp
//===----------------------------------------------------------------------===//

#include <common/reduction/Cluster2D.h>
#include <common/testutils/TestBase.h>

class Cluster2DTest : public TestBase {
protected:
  Cluster2D TestCluster, OtherCluster;
  double FPEquality{0.000001};

  void SetUp() override {}

  void TearDown() override {}
};


TEST_F(Cluster2DTest, Constructor) {
  EXPECT_EQ(TestCluster.xCoordEnd(), 0);
}


TEST_F(Cluster2DTest, IsValidIsEmpty) {
  ASSERT_FALSE(TestCluster.valid());
  ASSERT_TRUE(TestCluster.empty());

  Hit2D Zero2DHit{0,0,0,1};
  TestCluster.insert(Zero2DHit);
  ASSERT_TRUE(TestCluster.valid());
  ASSERT_FALSE(TestCluster.empty());
}


TEST_F(Cluster2DTest, MergeClusters) {
  EXPECT_NEAR(TestCluster.weightSum(), 0, FPEquality);

  OtherCluster.insert({0, 3, 0, 1});

  TestCluster.merge(OtherCluster);
  EXPECT_NEAR(TestCluster.weightSum(), 1, FPEquality);
}


TEST_F(Cluster2DTest, TimeOverlap) {
  EXPECT_EQ(TestCluster.timeOverlap(OtherCluster), 0);

  TestCluster.insert({0, 0, 0, 1});
  OtherCluster.insert({0, 7, 7, 1});

  EXPECT_EQ(TestCluster.timeOverlap(OtherCluster), 1);
}


TEST_F(Cluster2DTest, TimeGap) {

  TestCluster.insert({1, 0, 0, 1});
  OtherCluster.insert({0, 7, 7, 1});

  EXPECT_EQ(TestCluster.timeGap(OtherCluster), 1);

  TestCluster.insert({0, 0, 0, 1});
  EXPECT_EQ(TestCluster.timeGap(OtherCluster), 0);
}


TEST_F(Cluster2DTest, Clear) {
  Hit2D Zero2DHit{0,0,0,1};
  TestCluster.insert(Zero2DHit);
  EXPECT_NEAR(TestCluster.weightSum(), 1, FPEquality);
  TestCluster.clear();
  EXPECT_NEAR(TestCluster.weightSum(), 0, FPEquality);
}


TEST_F(Cluster2DTest, GetterFunctionsZero2DHit) {
  EXPECT_EQ(TestCluster.hitCount(), 0);

  Hit2D Zero2DHit{0,0,0,1};
  TestCluster.insert(Zero2DHit);

  EXPECT_EQ(TestCluster.hitCount(), 1);
  EXPECT_NEAR(TestCluster.timeEnd(), 0, FPEquality);
  EXPECT_NEAR(TestCluster.weightSum(), 1, FPEquality);
  EXPECT_NEAR(TestCluster.xCoordMass(), 0, FPEquality);
  EXPECT_NEAR(TestCluster.xCoordCenter(), 0, FPEquality);
  EXPECT_NEAR(TestCluster.yCoordMass(), 0, FPEquality);
  EXPECT_NEAR(TestCluster.yCoordCenter(), 0, FPEquality);
  EXPECT_NEAR(TestCluster.timeMass(), 0, FPEquality);
  EXPECT_NEAR(TestCluster.timeCenter(), 0, FPEquality);
  EXPECT_NEAR(TestCluster.xCoordMass2(), 0, FPEquality);
  EXPECT_NEAR(TestCluster.xCoordCenter2(), 0, FPEquality);
  EXPECT_NEAR(TestCluster.timeMass2(), 0, FPEquality);
  EXPECT_NEAR(TestCluster.timeCenter2(), 0, FPEquality);
}


TEST_F(Cluster2DTest, DebugString) {
  std::string DebugString = TestCluster.to_string("prefix_", true);

  ASSERT_TRUE(DebugString.size() > 0);
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
