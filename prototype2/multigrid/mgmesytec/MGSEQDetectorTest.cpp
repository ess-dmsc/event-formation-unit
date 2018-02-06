/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <libs/include/TSCTimer.h>
#include <multigrid/mgmesytec/MGSEQDetector.h>
#include <test/TestBase.h>

class MGSEQDetectorTest : public TestBase {};

/** Test cases below */
TEST_F(MGSEQDetectorTest, IsWireIsGrid) {
  MGSEQDetector mgdet;

  for (int i = 0; i <= 79; i++) {
    ASSERT_TRUE(mgdet.isWire(i));
    ASSERT_FALSE(mgdet.isGrid(i));
  }

  for (int i = 80; i <= 127; i++) {
    ASSERT_FALSE(mgdet.isWire(i));
    ASSERT_TRUE(mgdet.isGrid(i));
  }

  ASSERT_FALSE(mgdet.isWire(128));
  ASSERT_FALSE(mgdet.isGrid(128));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
