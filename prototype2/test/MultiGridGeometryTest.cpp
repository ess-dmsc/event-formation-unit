/** Copyright (C) 2016 European Spallation Source ERIC */

#include "TestBase.h"
#include <common/MultiGridGeometry.h>
#include <libs/include/TSCTimer.h>

using namespace std;

class MultiGridGeometryTest : public TestBase {};

/** Test cases below */
TEST_F(MultiGridGeometryTest, getdetectorpixelid) {
  MultiGridGeometry grid2x2(2,2,2,2);
  ASSERT_EQ( 1, grid2x2.getdetectorpixelid(1, 1, 1));
  ASSERT_EQ( 2, grid2x2.getdetectorpixelid(1, 1, 3));
  ASSERT_EQ( 3, grid2x2.getdetectorpixelid(2, 1, 1));
  ASSERT_EQ( 4, grid2x2.getdetectorpixelid(2, 1, 3));
  ASSERT_EQ( 5, grid2x2.getdetectorpixelid(1, 2, 1));
  ASSERT_EQ( 6, grid2x2.getdetectorpixelid(1, 2, 3));
  ASSERT_EQ( 7, grid2x2.getdetectorpixelid(2, 2, 1));
  ASSERT_EQ( 8, grid2x2.getdetectorpixelid(2, 2, 3));
  ASSERT_EQ( 9, grid2x2.getdetectorpixelid(1, 1, 2));
  ASSERT_EQ(10, grid2x2.getdetectorpixelid(1, 1, 4));
  ASSERT_EQ(11, grid2x2.getdetectorpixelid(2, 1, 2));
  ASSERT_EQ(12, grid2x2.getdetectorpixelid(2, 1, 4));
  ASSERT_EQ(13, grid2x2.getdetectorpixelid(1, 2, 2));
  ASSERT_EQ(14, grid2x2.getdetectorpixelid(1, 2, 4));
  ASSERT_EQ(15, grid2x2.getdetectorpixelid(2, 2, 2));
  ASSERT_EQ(16, grid2x2.getdetectorpixelid(2, 2, 4));
}


TEST_F(MultiGridGeometryTest, getdetectorpixelidCSPEC) {
  int cols = 80;
  int grids = 160;
  int xwires = 4;
  int zwires = 16;
  MultiGridGeometry gridCSPEC(cols, grids, xwires, zwires);
  MESSAGE() << "A few steps in the x-direction\n";
  ASSERT_EQ( 1, gridCSPEC.getdetectorpixelid(1, 1,            1));
  ASSERT_EQ( 2, gridCSPEC.getdetectorpixelid(1, 1, zwires   + 1));
  ASSERT_EQ( 3, gridCSPEC.getdetectorpixelid(1, 1, zwires*2 + 1));
  ASSERT_EQ( 4, gridCSPEC.getdetectorpixelid(1, 1, zwires*3 + 1));
  ASSERT_EQ( 5, gridCSPEC.getdetectorpixelid(2, 1,            1));

  MESSAGE() << "A few steps in the z-direction\n";
  ASSERT_EQ( cols * grids * xwires           + 1, gridCSPEC.getdetectorpixelid(1, 1, 2));
  ASSERT_EQ( cols * grids * xwires * (3 - 1) + 1, gridCSPEC.getdetectorpixelid(1, 1, 3));
  ASSERT_EQ( cols * grids * xwires * (4 - 1) + 1, gridCSPEC.getdetectorpixelid(1, 1, 4));
  ASSERT_EQ( cols * grids * xwires * (5 - 1) + 1, gridCSPEC.getdetectorpixelid(1, 1, 5));

  MESSAGE() << "A few steps in the y-direction\n";
  ASSERT_EQ(cols * xwires * (2 - 1) + 1, gridCSPEC.getdetectorpixelid(1,2,1));
  ASSERT_EQ(cols * xwires * (3 - 1) + 1, gridCSPEC.getdetectorpixelid(1,3,1));
  ASSERT_EQ(cols * xwires * (4 - 1) + 1, gridCSPEC.getdetectorpixelid(1,4,1));
  ASSERT_EQ(cols * xwires * (5 - 1) + 1, gridCSPEC.getdetectorpixelid(1,5,1));
  ASSERT_EQ(cols * xwires * (6 - 1) + 1, gridCSPEC.getdetectorpixelid(1,6,1));
}


TEST_F(MultiGridGeometryTest, Constructor) {
  for (int i = 2; i < 16; i++) {
    MultiGridGeometry gridNxN(i,i,i,i);
    ASSERT_EQ(gridNxN.getmaxdetectorid(), i*i*i*i);
  }
}

TEST_F(MultiGridGeometryTest, Bounds) {
  for (int i = 1; i < 16; i++) {
    MultiGridGeometry gridNxN(i,i,i,i);
    ASSERT_EQ(gridNxN.getdetectorpixelid(0,i,i*i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i,0,i*i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i,i,0), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i+1,i,i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i,i+1,i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i,i,i*i+1), -1);
  }
}


TEST_F(MultiGridGeometryTest, SpeedTest) {
  int cols = 80;
  int grids = 160;
  int xwires = 4;
  int zwires = 16;
  const uint64_t repeats = 1000000000; //1B
  uint64_t sum = 0;
  TSCTimer start;
  MultiGridGeometry CSPEC(cols, grids, xwires, zwires);
  for (unsigned int i = 1; i < repeats; i++) {
     sum += CSPEC.getdetectorpixelid(40 + i%31, 80, 37);
  }
  auto elapsed = start.timetsc();
  MESSAGE() << "100 x TSC Cycles/call : " << elapsed*100/repeats << std::endl;
  MESSAGE() << "SUM: " << sum << std::endl;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
