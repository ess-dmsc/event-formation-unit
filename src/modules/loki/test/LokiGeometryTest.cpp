/** Copyright (C) 2019-2022 European Spallation Source ERIC */

#include <algorithm>
#include <loki/geometry/LokiGeometry.h>
#include <common/testutils/TestBase.h>
#include <memory>

using namespace Caen;

class LokiGeometryTest : public TestBase {
protected:
  void SetUp() override {
    PanelGeometry Panel(4, 7, 0);
    CaenConfiguration.Panels.push_back(Panel);
  }
  void TearDown() override {}
  Config CaenConfiguration;
};

/** Test cases below */
TEST_F(LokiGeometryTest, Constructor) {
  LokiGeometry tube(CaenConfiguration);
  tube.setResolution(512);
  ASSERT_EQ(tube.StrawId, 7);  // valid: 0 - 6
  ASSERT_EQ(tube.PosVal, 512); // valid: 0 - 511
}

TEST_F(LokiGeometryTest, AllZeroes) {
  LokiGeometry tube(CaenConfiguration);
  tube.setResolution(512);
  tube.calcPositions(0, 0, 0, 0);
  ASSERT_EQ(tube.StrawId, 7);  // valid: 0 - 6
  ASSERT_EQ(tube.PosVal, 512); // valid: 0 - 511
  ASSERT_EQ(tube.Stats.AmplitudeZero, 1);
}

TEST_F(LokiGeometryTest, StrawLimits) {
  LokiGeometry tube(CaenConfiguration);
  double delta = 0.0001;
  ASSERT_EQ(tube.strawCalc(0.1), 0);
  ASSERT_EQ(tube.strawCalc(0.7 - delta), 0);
  ASSERT_EQ(tube.strawCalc(0.7), 0);
  ASSERT_EQ(tube.strawCalc(0.7 + delta), 1);
  ASSERT_EQ(tube.strawCalc(1.56 - delta), 1);
  ASSERT_EQ(tube.strawCalc(1.56), 1);
  ASSERT_EQ(tube.strawCalc(1.56 + delta), 2);
  ASSERT_EQ(tube.strawCalc(2.52 - delta), 2);
  ASSERT_EQ(tube.strawCalc(2.52), 2);
  ASSERT_EQ(tube.strawCalc(2.52 + delta), 3);
  ASSERT_EQ(tube.strawCalc(3.54 - delta), 3);
  ASSERT_EQ(tube.strawCalc(3.54), 3);
  ASSERT_EQ(tube.strawCalc(3.54 + delta), 4);
  ASSERT_EQ(tube.strawCalc(4.44 - delta), 4);
  ASSERT_EQ(tube.strawCalc(4.44), 4);
  ASSERT_EQ(tube.strawCalc(4.44 + delta), 5);
  ASSERT_EQ(tube.strawCalc(5.30 - delta), 5);
  ASSERT_EQ(tube.strawCalc(5.30), 5);
  ASSERT_EQ(tube.strawCalc(5.30 + delta), 6);
}

TEST_F(LokiGeometryTest, MinMaxStraw) {
  LokiGeometry tube(CaenConfiguration);
  tube.setResolution(512);
  unsigned int iMax = 4096;
  for (unsigned int i = 1; i < iMax; i++) {
    tube.calcPositions(0, i, 0, 0);
    ASSERT_EQ(tube.StrawId, 6);
    tube.calcPositions(0, 0, 0, i);
    ASSERT_EQ(tube.StrawId, 6);
    tube.calcPositions(0, i, 0, i);
    ASSERT_EQ(tube.StrawId, 6);

    tube.calcPositions(i, 0, i, 0);
    ASSERT_EQ(tube.StrawId, 0);
  }
}

TEST_F(LokiGeometryTest, MinMaxPos) {
  LokiGeometry tube(CaenConfiguration);
  tube.setResolution(512);
  for (unsigned int i = 1; i < 4095; i++) {
    tube.calcPositions(0, 0, i, 0);
    ASSERT_EQ(tube.PosVal, 0);
    tube.calcPositions(0, 0, 0, i);
    ASSERT_EQ(tube.PosVal, 0);
    tube.calcPositions(0, 0, i, i);
    ASSERT_EQ(tube.PosVal, 0);

    tube.calcPositions(i, i, 0, 0);
    ASSERT_EQ(tube.PosVal, 511);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
