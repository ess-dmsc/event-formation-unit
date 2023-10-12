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
  Cluster2D TestCluster;
  double FPEquality{0.000001};

  void SetUp() override {}

  void TearDown() override {}
};


TEST_F(Cluster2DTest, Constructor) {
  EXPECT_EQ(TestCluster.xCoordEnd(), 0);
}


TEST_F(Cluster2DTest, GetterFunctionsConstructor) {
  // Division by zero for some functions if weights are 0, hence the insert
  Hit2D Zero2DHit{0,0,0,1};
  TestCluster.insert(Zero2DHit);

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
