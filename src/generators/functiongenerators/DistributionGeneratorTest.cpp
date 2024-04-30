// Copyright (C) 2019-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for ESS related time objects and functions
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <generators/functiongenerators/DistributionGenerator.h>

class DistributionGeneratorTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------

///\brief not the best of test, just checking end of ranges with current
// known values
TEST_F(DistributionGeneratorTest, Constructors) {
  DistributionGenerator MyDist(1000.0/14);
  ASSERT_NEAR(MyDist.Dist[0], 0.001, 1e-4);
  ASSERT_NEAR(MyDist.Dist[MyDist.Bins - 1], 0.001, 1e-4);
  ASSERT_NEAR(MyDist.CDF[0], 0.000, 1e-4);
  ASSERT_NEAR(MyDist.CDF[1], 0.001, 1e-4);
  ASSERT_NEAR(MyDist.CDF[MyDist.Bins - 1], 11.957, 0.005);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
