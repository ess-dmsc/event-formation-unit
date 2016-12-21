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


TEST_F(MultiGridGeometryTest, UndersizeOversizeGeometry) {
  MultiGridGeometry p2m2g2_2x2(2, 2, 2, 2, 2, 0, 0);

  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(0, 1, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 0, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 1, 0));

  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(3, 1, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 5, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 1, 9));
}


TEST_F(MultiGridGeometryTest, UndersizeOversizeGeometryWOffset) {
  MultiGridGeometry p2m2g2_2x2_off(2, 2, 2, 2, 2, 1, 0);

  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(-1, 1, 1));
  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(1, 0, 1));
  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(1, 1, 0));

  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(2, 1, 1));
  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(1, 5, 1));
  ASSERT_EQ(-1, p2m2g2_2x2_off.getdetectorpixelid(1, 1, 9));
}

TEST_F(MultiGridGeometryTest, OnePanelGetPixelID) {
  MultiGridGeometry p1m2g2_2x2(1, 2, 2, 2, 2, 0, 0);
  ASSERT_EQ(16, p1m2g2_2x2.getmaxpixelid());

  ASSERT_EQ( 1, p1m2g2_2x2.getdetectorpixelid(1, 2, 1));
  ASSERT_EQ( 2, p1m2g2_2x2.getdetectorpixelid(1, 2, 2));
  ASSERT_EQ( 3, p1m2g2_2x2.getdetectorpixelid(1, 1, 1));
  ASSERT_EQ( 4, p1m2g2_2x2.getdetectorpixelid(1, 1, 2));

  ASSERT_EQ( 5, p1m2g2_2x2.getdetectorpixelid(1, 2, 3));
  ASSERT_EQ( 6, p1m2g2_2x2.getdetectorpixelid(1, 2, 4));
  ASSERT_EQ( 7, p1m2g2_2x2.getdetectorpixelid(1, 1, 3));
  ASSERT_EQ( 8, p1m2g2_2x2.getdetectorpixelid(1, 1, 4));

  ASSERT_EQ( 9, p1m2g2_2x2.getdetectorpixelid(1, 4, 5));
  ASSERT_EQ(10, p1m2g2_2x2.getdetectorpixelid(1, 4, 6));
  ASSERT_EQ(11, p1m2g2_2x2.getdetectorpixelid(1, 3, 5));
  ASSERT_EQ(12, p1m2g2_2x2.getdetectorpixelid(1, 3, 6));

  ASSERT_EQ(13, p1m2g2_2x2.getdetectorpixelid(1, 4, 7));
  ASSERT_EQ(14, p1m2g2_2x2.getdetectorpixelid(1, 4, 8));
  ASSERT_EQ(15, p1m2g2_2x2.getdetectorpixelid(1, 3, 7));
  ASSERT_EQ(16, p1m2g2_2x2.getdetectorpixelid(1, 3, 8));
}



TEST_F(MultiGridGeometryTest, TwoPanelsGetPixelID) {
  MultiGridGeometry p2m2g2_2x2(2, 2, 2, 2, 2, 0, 0);
  ASSERT_EQ(32, p2m2g2_2x2.getmaxpixelid());

  ASSERT_EQ( 1, p2m2g2_2x2.getdetectorpixelid(1, 2, 1));
  ASSERT_EQ(16, p2m2g2_2x2.getdetectorpixelid(1, 3, 8));

  ASSERT_EQ(17, p2m2g2_2x2.getdetectorpixelid(2, 2, 1));
  ASSERT_EQ(18, p2m2g2_2x2.getdetectorpixelid(2, 2, 2));
  ASSERT_EQ(19, p2m2g2_2x2.getdetectorpixelid(2, 1, 1));
  ASSERT_EQ(20, p2m2g2_2x2.getdetectorpixelid(2, 1, 2));
  ASSERT_EQ(21, p2m2g2_2x2.getdetectorpixelid(2, 2, 3));
  ASSERT_EQ(22, p2m2g2_2x2.getdetectorpixelid(2, 2, 4));
  ASSERT_EQ(23, p2m2g2_2x2.getdetectorpixelid(2, 1, 3));
  ASSERT_EQ(24, p2m2g2_2x2.getdetectorpixelid(2, 1, 4));
  ASSERT_EQ(25, p2m2g2_2x2.getdetectorpixelid(2, 4, 5));
  ASSERT_EQ(26, p2m2g2_2x2.getdetectorpixelid(2, 4, 6));
  ASSERT_EQ(27, p2m2g2_2x2.getdetectorpixelid(2, 3, 5));
  ASSERT_EQ(28, p2m2g2_2x2.getdetectorpixelid(2, 3, 6));
  ASSERT_EQ(29, p2m2g2_2x2.getdetectorpixelid(2, 4, 7));
  ASSERT_EQ(30, p2m2g2_2x2.getdetectorpixelid(2, 4, 8));
  ASSERT_EQ(31, p2m2g2_2x2.getdetectorpixelid(2, 3, 7));
  ASSERT_EQ(32, p2m2g2_2x2.getdetectorpixelid(2, 3, 8));
}

TEST_F(MultiGridGeometryTest, TwoPanelsGetPixelIDWOffset) {
  MultiGridGeometry p2m2g2_2x2(2, 2, 2, 2, 2, 1, 0);

  ASSERT_EQ(32, p2m2g2_2x2.getmaxpixelid());

  ASSERT_EQ( 1, p2m2g2_2x2.getdetectorpixelid(0, 2, 1));
  ASSERT_EQ(16, p2m2g2_2x2.getdetectorpixelid(0, 3, 8));
  ASSERT_EQ(17, p2m2g2_2x2.getdetectorpixelid(1, 2, 1));

  ASSERT_EQ(32, p2m2g2_2x2.getdetectorpixelid(1, 3, 8));
}

TEST_F(MultiGridGeometryTest, TwoPanelsSwappedWires) {
  MultiGridGeometry p2m2g2_2x2(2, 2, 2, 2, 2, 0, 1);

  ASSERT_EQ(32, p2m2g2_2x2.getmaxpixelid());

  ASSERT_EQ(2, p2m2g2_2x2.getdetectorpixelid(1, 2, 1));
  ASSERT_EQ(1, p2m2g2_2x2.getdetectorpixelid(1, 2, 2));
  ASSERT_EQ(16, p2m2g2_2x2.getdetectorpixelid(1, 3, 7));
  ASSERT_EQ(15, p2m2g2_2x2.getdetectorpixelid(1, 3, 8));

  ASSERT_EQ(18, p2m2g2_2x2.getdetectorpixelid(2, 2, 1));
  ASSERT_EQ(17, p2m2g2_2x2.getdetectorpixelid(2, 2, 2));
  ASSERT_EQ(32, p2m2g2_2x2.getdetectorpixelid(2, 3, 7));
  ASSERT_EQ(31, p2m2g2_2x2.getdetectorpixelid(2, 3, 8));
}


TEST_F(MultiGridGeometryTest, CSPECGetPixelID) {
  int panels = 1;
  int modules = 80;
  int grids = 160;
  int xwires = 4;
  int zwires = 16;
  MultiGridGeometry gridCSPEC(panels, modules, grids, xwires, zwires, 0, 0);
  MESSAGE() << "This test sucks, please improve\n";
  MESSAGE() << "A few steps in the z-direction\n";
  ASSERT_EQ(1, gridCSPEC.getdetectorpixelid(1, grids, 1));
  ASSERT_EQ(2, gridCSPEC.getdetectorpixelid(1, grids, 2));
  ASSERT_EQ(3, gridCSPEC.getdetectorpixelid(1, grids, 3));
  ASSERT_EQ(4, gridCSPEC.getdetectorpixelid(1, grids, 4));
  ASSERT_EQ(5, gridCSPEC.getdetectorpixelid(1, grids, 5));

  MESSAGE() << "A few steps in the x-direction\n";
  ASSERT_EQ(    grids * zwires + 1, gridCSPEC.getdetectorpixelid(1, grids, zwires * 1 + 1));
  ASSERT_EQ(2 * grids * zwires + 1, gridCSPEC.getdetectorpixelid(1, grids, zwires * 2 + 1));
  ASSERT_EQ(3 * grids * zwires + 1, gridCSPEC.getdetectorpixelid(1, grids, zwires * 3 + 1));
  //ASSERT_EQ(4 * grids * zwires + 1, gridCSPEC.getdetectorpixelid(1, grids, zwires * 4 + 1));

  MESSAGE() << "A few steps in the y-direction\n";
  ASSERT_EQ(1, gridCSPEC.getdetectorpixelid(1, grids    , 1));
  ASSERT_EQ(zwires  + 1, gridCSPEC.getdetectorpixelid(1, grids - 1, 1));
}


TEST_F(MultiGridGeometryTest, NxNConstructor) {
  for (int i = 2; i < 16; i++) {
    MultiGridGeometry gridNxN(i, i, i, i, i, 0, 0);
    ASSERT_EQ(gridNxN.getmaxpixelid(), i * i * i * i * i);
  }
}

TEST_F(MultiGridGeometryTest, NxNGridBounds) {
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

TEST_F(MultiGridGeometryTest, CSPECDetectorSpeedTest) {
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
    sum += CSPEC.getdetectorpixelid(7 % i + 1, 40 + i % 31, 37);
  }
  auto elapsed = start.timetsc();
  MESSAGE() << "100 x TSC Cycles/call : " << elapsed * 100 / repeats
            << std::endl;
  MESSAGE() << "SUM: " << sum << std::endl;
}

TEST_F(MultiGridGeometryTest, TestDetectorModuleBoundaries) {
  int panels = 1;
  int modules = 2;
  int grids = 48;
  int xwires = 4;
  int zwires = 16;
  MultiGridGeometry testdetector(panels, modules, grids, xwires, zwires, 1, 0);

  ASSERT_EQ(   1, testdetector.getdetectorpixelid(0, 48,  1));
  ASSERT_EQ(  16, testdetector.getdetectorpixelid(0, 48, 16));
  ASSERT_EQ(2305, testdetector.getdetectorpixelid(0, 48, 49));
  ASSERT_EQ(2320, testdetector.getdetectorpixelid(0, 48, 64));

  ASSERT_EQ( 753, testdetector.getdetectorpixelid(0,  1,  1));
  ASSERT_EQ( 768, testdetector.getdetectorpixelid(0,  1, 16));
  ASSERT_EQ(3057, testdetector.getdetectorpixelid(0,  1, 49));
  ASSERT_EQ(3072, testdetector.getdetectorpixelid(0,  1, 64));

  ASSERT_EQ(3073, testdetector.getdetectorpixelid(0, 96, 65));
  ASSERT_EQ(3088, testdetector.getdetectorpixelid(0, 96, 80));
  ASSERT_EQ(5377, testdetector.getdetectorpixelid(0, 96,113));
  ASSERT_EQ(5392, testdetector.getdetectorpixelid(0, 96,128));

  ASSERT_EQ(3825, testdetector.getdetectorpixelid(0, 49, 65));
  ASSERT_EQ(3840, testdetector.getdetectorpixelid(0, 49, 80));
  ASSERT_EQ(6129, testdetector.getdetectorpixelid(0, 49,113));
  ASSERT_EQ(6144, testdetector.getdetectorpixelid(0, 49,128));
}


TEST_F(MultiGridGeometryTest, TestDetectorIllegalWireAndGridid) {
  int panels = 1;
  int modules = 2;
  int grids = 48;
  int xwires = 4;
  int zwires = 16;
  MultiGridGeometry testdetector(panels, modules, grids, xwires, zwires, 1, 0);

  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 49,   1));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 96,   1));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 49,  64));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 96,  64));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1,  1,  65));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 48,  65));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1,  1, 128));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 48, 128));
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
