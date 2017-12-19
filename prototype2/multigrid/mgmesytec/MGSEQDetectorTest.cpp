/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <libs/include/TSCTimer.h>
#include <multigrid/mgmesytec/MGSEQDetector.h>
#include <test/TestBase.h>

class MGSEQDetectorTest : public TestBase {};

/** Test cases below */
TEST_F(MGSEQDetectorTest, IsWireIsGrid) {
  MGSEQDetector mgdet;

  ASSERT_FALSE(mgdet.isWire(0));
  ASSERT_FALSE(mgdet.isGrid(0));

  for (int i = 1; i < 81; i++) {
    ASSERT_TRUE(mgdet.isWire(i));
    ASSERT_FALSE(mgdet.isGrid(i));
  }

  for (int i = 81; i < 93; i++) {
    ASSERT_FALSE(mgdet.isWire(i));
    ASSERT_TRUE(mgdet.isGrid(i));
  }

  ASSERT_FALSE(mgdet.isWire(93));
  ASSERT_FALSE(mgdet.isGrid(93));
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
