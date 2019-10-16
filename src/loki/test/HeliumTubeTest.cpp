/** Copyright (C) 2019 European Spallation Source ERIC */

#include <algorithm>
#include <memory>
#include <loki/geometry/HeliumTube.h>
#include <test/TestBase.h>

using namespace Loki;

class HeliumTubeTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(HeliumTubeTest, Constructor) {
  HeliumTube tube;
  ASSERT_EQ(tube.StrawId, 7); // valid: 0 - 6
  ASSERT_EQ(tube.PosId, 512); // valid: 0 - 511
}

TEST_F(HeliumTubeTest, AllZeroes) {
  HeliumTube tube;
  tube.calcPositions(0,0,0,0);
  ASSERT_EQ(tube.StrawId, 7); // valid: 0 - 6
  ASSERT_EQ(tube.PosId, 512); // valid: 0 - 511
  ASSERT_EQ(tube.Stats.AmplitudeZero, 1);
}

TEST_F(HeliumTubeTest, MinMaxStraw) {
  HeliumTube tube;
  unsigned int iMax = 4096;
  for (unsigned int i = 1; i < iMax; i++) {
    tube.calcPositions(0,0,i,0);
    ASSERT_EQ(tube.StrawId, 0);
    tube.calcPositions(0,0,0,i);
    ASSERT_EQ(tube.StrawId, 0);
    tube.calcPositions(0,0,i,i);
    ASSERT_EQ(tube.StrawId, 0);

    tube.calcPositions(i,i,0,0);
    ASSERT_EQ(tube.StrawId, 6);
  }
}

TEST_F(HeliumTubeTest, MinMaxPos) {
  HeliumTube tube;
  for (unsigned int i = 1; i < 4095; i++) {
    tube.calcPositions(0,i,0,0);
    ASSERT_EQ(tube.PosId, 0);
    tube.calcPositions(0,0,i,0);
    ASSERT_EQ(tube.PosId, 0);
    tube.calcPositions(0,i,i,0);
    ASSERT_EQ(tube.PosId, 0);

    tube.calcPositions(i,0,0,i);
    ASSERT_EQ(tube.PosId, 511);
  }
}
