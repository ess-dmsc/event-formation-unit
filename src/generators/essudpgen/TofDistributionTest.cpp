// Copyright (C) 2019-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for ESS related time objects and functions
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <generators/essudpgen/TofDistribution.h>

class TofDistrubutionTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------

TEST_F(TofDistrubutionTest, Constructors) {
  TofDistribution MyDist;
  printf("%.2f\n", MyDist.Norm);
  for (int i = 0; i < 10; i++) {
    double val = MyDist.getRandomTof();
    printf("%.2f\n", val);
  }
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
