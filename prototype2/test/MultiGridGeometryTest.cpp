/** Copyright (C) 2016 European Spallation Source ERIC */

#include "TestBase.h"
#include <common/MultiGridGeometry.h>
#include <libs/include/TSCTimer.h>

using namespace std;

class MultiGridGeometryTest : public TestBase {};

/** Test cases below */
TEST_F(MultiGridGeometryTest, getmaxpixelid) {
  MultiGridGeometry p1m1grid1x1(1, 1, 1, 1, 1, 0, 0);
  ASSERT_EQ(1, p1m1grid1x1.getmaxpixelid());

  MultiGridGeometry p1m2grid2x2(1, 2, 2, 2, 2, 0, 0);
  ASSERT_EQ(16, p1m2grid2x2.getmaxpixelid());

  MultiGridGeometry cncs(1, 2, 48, 4, 16, 0, 0);
  ASSERT_EQ(6144, cncs.getmaxpixelid());
}

TEST_F(MultiGridGeometryTest, underoversize) {
  MultiGridGeometry p2m2g2_2x2(2, 2, 2, 2, 2, 0, 0);

  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(0, 1, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 0, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 1, 0));

  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(3, 1, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 5, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 1, 9));
}

TEST_F(MultiGridGeometryTest, underoversize_offset) {
  MultiGridGeometry p2m2g2_2x2_off(2, 2, 2, 2, 2, 1, 0);

  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(-1, 1, 1));
  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(1, 0, 1));
  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(1, 1, 0));

  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(2, 1, 1));
  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(1, 5, 1));
  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(1, 1, 9));
}

TEST_F(MultiGridGeometryTest, getpixelid_one_panel) {
  MultiGridGeometry p1m2g2_2x2(1, 2, 2, 2, 2, 0, 0);

  ASSERT_EQ(1, p1m2g2_2x2.getdetectorpixelid(1, 1, 1));
  ASSERT_EQ(2, p1m2g2_2x2.getdetectorpixelid(1, 1, 2));
  ASSERT_EQ(3, p1m2g2_2x2.getdetectorpixelid(1, 2, 1));
  ASSERT_EQ(4, p1m2g2_2x2.getdetectorpixelid(1, 2, 2));

  ASSERT_EQ(5, p1m2g2_2x2.getdetectorpixelid(1, 1, 3));
  ASSERT_EQ(6, p1m2g2_2x2.getdetectorpixelid(1, 1, 4));
  ASSERT_EQ(7, p1m2g2_2x2.getdetectorpixelid(1, 2, 3));
  ASSERT_EQ(8, p1m2g2_2x2.getdetectorpixelid(1, 2, 4));
  ASSERT_EQ(9, p1m2g2_2x2.getdetectorpixelid(1, 1, 5));
  ASSERT_EQ(10, p1m2g2_2x2.getdetectorpixelid(1, 1, 6));
  ASSERT_EQ(11, p1m2g2_2x2.getdetectorpixelid(1, 2, 5));
  ASSERT_EQ(12, p1m2g2_2x2.getdetectorpixelid(1, 2, 6));
  ASSERT_EQ(13, p1m2g2_2x2.getdetectorpixelid(1, 1, 7));
  ASSERT_EQ(14, p1m2g2_2x2.getdetectorpixelid(1, 1, 8));
  ASSERT_EQ(15, p1m2g2_2x2.getdetectorpixelid(1, 2, 7));
  ASSERT_EQ(16, p1m2g2_2x2.getdetectorpixelid(1, 2, 8));
}

TEST_F(MultiGridGeometryTest, getpixelid_two_panels) {
  MultiGridGeometry p2m2g2_2x2(2, 2, 2, 2, 2, 0, 0);
  ASSERT_EQ(32, p2m2g2_2x2.getmaxpixelid());

  ASSERT_EQ(1, p2m2g2_2x2.getdetectorpixelid(1, 1, 1));
  ASSERT_EQ(16, p2m2g2_2x2.getdetectorpixelid(1, 2, 8));

  ASSERT_EQ(17, p2m2g2_2x2.getdetectorpixelid(2, 1, 1));
  ASSERT_EQ(18, p2m2g2_2x2.getdetectorpixelid(2, 1, 2));
  ASSERT_EQ(19, p2m2g2_2x2.getdetectorpixelid(2, 2, 1));
  ASSERT_EQ(20, p2m2g2_2x2.getdetectorpixelid(2, 2, 2));
  ASSERT_EQ(21, p2m2g2_2x2.getdetectorpixelid(2, 1, 3));
  ASSERT_EQ(22, p2m2g2_2x2.getdetectorpixelid(2, 1, 4));
  ASSERT_EQ(23, p2m2g2_2x2.getdetectorpixelid(2, 2, 3));
  ASSERT_EQ(24, p2m2g2_2x2.getdetectorpixelid(2, 2, 4));
  ASSERT_EQ(25, p2m2g2_2x2.getdetectorpixelid(2, 1, 5));
  ASSERT_EQ(26, p2m2g2_2x2.getdetectorpixelid(2, 1, 6));
  ASSERT_EQ(27, p2m2g2_2x2.getdetectorpixelid(2, 2, 5));
  ASSERT_EQ(28, p2m2g2_2x2.getdetectorpixelid(2, 2, 6));
  ASSERT_EQ(29, p2m2g2_2x2.getdetectorpixelid(2, 1, 7));
  ASSERT_EQ(30, p2m2g2_2x2.getdetectorpixelid(2, 1, 8));
  ASSERT_EQ(31, p2m2g2_2x2.getdetectorpixelid(2, 2, 7));
  ASSERT_EQ(32, p2m2g2_2x2.getdetectorpixelid(2, 2, 8));
}

TEST_F(MultiGridGeometryTest, one_panel_offset) {
  MultiGridGeometry p2m2g2_2x2(2, 2, 2, 2, 2, 1, 0);

  ASSERT_EQ(32, p2m2g2_2x2.getmaxpixelid());

  ASSERT_EQ(1, p2m2g2_2x2.getdetectorpixelid(0, 1, 1));
  ASSERT_EQ(16, p2m2g2_2x2.getdetectorpixelid(0, 2, 8));
  ASSERT_EQ(17, p2m2g2_2x2.getdetectorpixelid(1, 1, 1));
  ASSERT_EQ(32, p2m2g2_2x2.getdetectorpixelid(1, 2, 8));
}

TEST_F(MultiGridGeometryTest, one_panel_swapwires) {
  MultiGridGeometry p2m2g2_2x2(2, 2, 2, 2, 2, 0, 1);

  ASSERT_EQ(32, p2m2g2_2x2.getmaxpixelid());

  ASSERT_EQ(2, p2m2g2_2x2.getdetectorpixelid(1, 1, 1));
  ASSERT_EQ(1, p2m2g2_2x2.getdetectorpixelid(1, 1, 2));
  ASSERT_EQ(16, p2m2g2_2x2.getdetectorpixelid(1, 2, 7));
  ASSERT_EQ(15, p2m2g2_2x2.getdetectorpixelid(1, 2, 8));

  ASSERT_EQ(18, p2m2g2_2x2.getdetectorpixelid(2, 1, 1));
  ASSERT_EQ(17, p2m2g2_2x2.getdetectorpixelid(2, 1, 2));
  ASSERT_EQ(32, p2m2g2_2x2.getdetectorpixelid(2, 2, 7));
  ASSERT_EQ(31, p2m2g2_2x2.getdetectorpixelid(2, 2, 8));
}

#if 0
TEST_F(MultiGridGeometryTest, getdetectorpixelidCSPEC) {
  int cols = 80;
  int grids = 160;
  int xwires = 4;
  int zwires = 16;
  MultiGridGeometry gridCSPEC(cols, grids, xwires, zwires);
  MESSAGE() << "A few steps in the x-direction\n";
  ASSERT_EQ(1, gridCSPEC.getdetectorpixelid(1, 1, 1));
  ASSERT_EQ(2, gridCSPEC.getdetectorpixelid(1, 1, zwires + 1));
  ASSERT_EQ(3, gridCSPEC.getdetectorpixelid(1, 1, zwires * 2 + 1));
  ASSERT_EQ(4, gridCSPEC.getdetectorpixelid(1, 1, zwires * 3 + 1));
  ASSERT_EQ(5, gridCSPEC.getdetectorpixelid(2, 1, 1));

  MESSAGE() << "A few steps in the z-direction\n";
  ASSERT_EQ(cols * grids * xwires + 1, gridCSPEC.getdetectorpixelid(1, 1, 2));
  ASSERT_EQ(cols * grids * xwires * (3 - 1) + 1,
            gridCSPEC.getdetectorpixelid(1, 1, 3));
  ASSERT_EQ(cols * grids * xwires * (4 - 1) + 1,
            gridCSPEC.getdetectorpixelid(1, 1, 4));
  ASSERT_EQ(cols * grids * xwires * (5 - 1) + 1,
            gridCSPEC.getdetectorpixelid(1, 1, 5));

  MESSAGE() << "A few steps in the y-direction\n";
  ASSERT_EQ(cols * xwires * (2 - 1) + 1, gridCSPEC.getdetectorpixelid(1, 2, 1));
  ASSERT_EQ(cols * xwires * (3 - 1) + 1, gridCSPEC.getdetectorpixelid(1, 3, 1));
  ASSERT_EQ(cols * xwires * (4 - 1) + 1, gridCSPEC.getdetectorpixelid(1, 4, 1));
  ASSERT_EQ(cols * xwires * (5 - 1) + 1, gridCSPEC.getdetectorpixelid(1, 5, 1));
  ASSERT_EQ(cols * xwires * (6 - 1) + 1, gridCSPEC.getdetectorpixelid(1, 6, 1));
}
#endif

TEST_F(MultiGridGeometryTest, Constructor) {
  for (int i = 2; i < 16; i++) {
    MultiGridGeometry gridNxN(i, i, i, i, i, 0, 0);
    ASSERT_EQ(gridNxN.getmaxpixelid(), i * i * i * i * i);
  }
}

TEST_F(MultiGridGeometryTest, Bounds) {
  for (int i = 1; i < 16; i++) {
    MultiGridGeometry gridNxN(i, i, i, i, i, 0, 0);
    ASSERT_EQ(gridNxN.getdetectorpixelid(0, i, i * i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i, 0, i * i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i, i, 0), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i + 1, i, i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i, i + 1, i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i, i, i * i * i + 1), -1);
  }
}

TEST_F(MultiGridGeometryTest, SpeedTest) {
  int panels = 8;
  int modules = 10;
  int grids = 160;
  int xwires = 4;
  int zwires = 16;
  // const uint64_t repeats = 1000000000; //1B too much for Valgrind
  int repeats = 10000000;
  uint64_t sum = 0;
  TSCTimer start;
  MultiGridGeometry CSPEC(panels, modules, grids, xwires, zwires, 0, 0);
  for (int i = 1; i < repeats; i++) {
    sum += CSPEC.getdetectorpixelid(8 % i + 1, 40 + i % 31, 37);
  }
  auto elapsed = start.timetsc();
  MESSAGE() << "100 x TSC Cycles/call : " << elapsed * 100 / repeats
            << std::endl;
  MESSAGE() << "SUM: " << sum << std::endl;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
