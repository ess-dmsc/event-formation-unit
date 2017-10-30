/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <libs/include/TSCTimer.h>
#include <multigrid/mgcncs/MultigridGeometry.h>
#include <test/TestBase.h>

class MultiGridGeometryTest : public TestBase {};

/** Test cases below */
TEST_F(MultiGridGeometryTest, getmaxpixelid) {
  MultiGridGeometry p1m1grid1x1(1, 1, 1, 1, 1);
  ASSERT_EQ(1, p1m1grid1x1.getmaxpixelid());

  MultiGridGeometry p1m2grid2x2(1, 2, 2, 2, 2);
  ASSERT_EQ(16, p1m2grid2x2.getmaxpixelid());

  MultiGridGeometry cncs(1, 2, 48, 4, 16);
  ASSERT_EQ(6144, cncs.getmaxpixelid());
}

TEST_F(MultiGridGeometryTest, UndersizeOversizeGeometry) {
  MultiGridGeometry p2m2g2_2x2(2, 2, 2, 2, 2);

  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(0, 1, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 0, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 1, 0));

  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(3, 1, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 5, 1));
  ASSERT_EQ(-1, p2m2g2_2x2.getdetectorpixelid(1, 1, 9));
}

TEST_F(MultiGridGeometryTest, OnePanelGetPixelID) {
  MultiGridGeometry p1m2g2_2x2(1, 2, 2, 2, 2);
  ASSERT_EQ(16, p1m2g2_2x2.getmaxpixelid());

  ASSERT_EQ(1, p1m2g2_2x2.getdetectorpixelid(1, 1, 7));
  ASSERT_EQ(2, p1m2g2_2x2.getdetectorpixelid(1, 1, 8));
  ASSERT_EQ(3, p1m2g2_2x2.getdetectorpixelid(1, 2, 7));
  ASSERT_EQ(4, p1m2g2_2x2.getdetectorpixelid(1, 2, 8));

  ASSERT_EQ(5, p1m2g2_2x2.getdetectorpixelid(1, 1, 5));
  ASSERT_EQ(6, p1m2g2_2x2.getdetectorpixelid(1, 1, 6));
  ASSERT_EQ(7, p1m2g2_2x2.getdetectorpixelid(1, 2, 5));
  ASSERT_EQ(8, p1m2g2_2x2.getdetectorpixelid(1, 2, 6));

  ASSERT_EQ(9, p1m2g2_2x2.getdetectorpixelid(1, 3, 3));
  ASSERT_EQ(10, p1m2g2_2x2.getdetectorpixelid(1, 3, 4));
  ASSERT_EQ(11, p1m2g2_2x2.getdetectorpixelid(1, 4, 3));
  ASSERT_EQ(12, p1m2g2_2x2.getdetectorpixelid(1, 4, 4));

  ASSERT_EQ(13, p1m2g2_2x2.getdetectorpixelid(1, 3, 1));
  ASSERT_EQ(14, p1m2g2_2x2.getdetectorpixelid(1, 3, 2));
  ASSERT_EQ(15, p1m2g2_2x2.getdetectorpixelid(1, 4, 1));
  ASSERT_EQ(16, p1m2g2_2x2.getdetectorpixelid(1, 4, 2));
}

TEST_F(MultiGridGeometryTest, TwoPanelsGetPixelID) {
  MultiGridGeometry p2m2g2_2x2(2, 2, 2, 2, 2);
  ASSERT_EQ(32, p2m2g2_2x2.getmaxpixelid());

  ASSERT_EQ(1, p2m2g2_2x2.getdetectorpixelid(1, 1, 7));
  ASSERT_EQ(16, p2m2g2_2x2.getdetectorpixelid(1, 4, 2));

  ASSERT_EQ(17, p2m2g2_2x2.getdetectorpixelid(2, 1, 7));
  ASSERT_EQ(18, p2m2g2_2x2.getdetectorpixelid(2, 1, 8));
  ASSERT_EQ(19, p2m2g2_2x2.getdetectorpixelid(2, 2, 7));
  ASSERT_EQ(20, p2m2g2_2x2.getdetectorpixelid(2, 2, 8));
  ASSERT_EQ(21, p2m2g2_2x2.getdetectorpixelid(2, 1, 5));
  ASSERT_EQ(22, p2m2g2_2x2.getdetectorpixelid(2, 1, 6));
  ASSERT_EQ(23, p2m2g2_2x2.getdetectorpixelid(2, 2, 5));
  ASSERT_EQ(24, p2m2g2_2x2.getdetectorpixelid(2, 2, 6));
  ASSERT_EQ(25, p2m2g2_2x2.getdetectorpixelid(2, 3, 3));
  ASSERT_EQ(26, p2m2g2_2x2.getdetectorpixelid(2, 3, 4));
  ASSERT_EQ(27, p2m2g2_2x2.getdetectorpixelid(2, 4, 3));
  ASSERT_EQ(28, p2m2g2_2x2.getdetectorpixelid(2, 4, 4));
  ASSERT_EQ(29, p2m2g2_2x2.getdetectorpixelid(2, 3, 1));
  ASSERT_EQ(30, p2m2g2_2x2.getdetectorpixelid(2, 3, 2));
  ASSERT_EQ(31, p2m2g2_2x2.getdetectorpixelid(2, 4, 1));
  ASSERT_EQ(32, p2m2g2_2x2.getdetectorpixelid(2, 4, 2));
}

TEST_F(MultiGridGeometryTest, NxNConstructor) {
  for (int i = 2; i < 16; i++) {
    MultiGridGeometry gridNxN(i, i, i, i, i);
    ASSERT_EQ(gridNxN.getmaxpixelid(), i * i * i * i * i);
  }
}

TEST_F(MultiGridGeometryTest, NxNGridBounds) {
  for (int i = 1; i < 16; i++) {
    MESSAGE() << "i " << i << "\n";
    MultiGridGeometry gridNxN(i, i, i, i, i);
    ASSERT_EQ(gridNxN.getdetectorpixelid(0, i, i * i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i, 0, i * i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i, i, 0), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i + 1, i, i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i, i * i + 1, i), -1);
    ASSERT_EQ(gridNxN.getdetectorpixelid(i, i, i * i * i + 1), -1);
  }
}

TEST_F(MultiGridGeometryTest, TestDetectorModuleBoundaries) {
  int panels = 1;
  int modules = 2;
  int grids = 48;
  int xwires = 4;
  int zwires = 16;
  MultiGridGeometry testdetector(panels, modules, grids, xwires, zwires);

  ASSERT_EQ(1, testdetector.getdetectorpixelid(1, 1, 113));
  ASSERT_EQ(16, testdetector.getdetectorpixelid(1, 1, 128));
  ASSERT_EQ(2305, testdetector.getdetectorpixelid(1, 1, 65));
  ASSERT_EQ(2320, testdetector.getdetectorpixelid(1, 1, 80));

  ASSERT_EQ(753, testdetector.getdetectorpixelid(1, 48, 113));
  ASSERT_EQ(768, testdetector.getdetectorpixelid(1, 48, 128));
  ASSERT_EQ(3057, testdetector.getdetectorpixelid(1, 48, 65));
  ASSERT_EQ(3072, testdetector.getdetectorpixelid(1, 48, 80));

  ASSERT_EQ(3073, testdetector.getdetectorpixelid(1, 49, 49));
  ASSERT_EQ(3088, testdetector.getdetectorpixelid(1, 49, 64));
  ASSERT_EQ(5377, testdetector.getdetectorpixelid(1, 49, 1));
  ASSERT_EQ(5392, testdetector.getdetectorpixelid(1, 49, 16));

  ASSERT_EQ(3825, testdetector.getdetectorpixelid(1, 96, 49));
  ASSERT_EQ(3840, testdetector.getdetectorpixelid(1, 96, 64));
  ASSERT_EQ(6129, testdetector.getdetectorpixelid(1, 96, 1));
  ASSERT_EQ(6144, testdetector.getdetectorpixelid(1, 96, 16));
}

TEST_F(MultiGridGeometryTest, Modules) {
  int panels = 1;
  int modules = 4;
  int grids = 2;
  int xwires = 2;
  int zwires = 2;
  MultiGridGeometry testdetector(panels, modules, grids, xwires, zwires);
  ASSERT_EQ(1, testdetector.getdetectorpixelid(1, 1, 15));
  ASSERT_EQ(2, testdetector.getdetectorpixelid(1, 1, 16));
  ASSERT_EQ(3, testdetector.getdetectorpixelid(1, 2, 15));
  ASSERT_EQ(4, testdetector.getdetectorpixelid(1, 2, 16));
  ASSERT_EQ(5, testdetector.getdetectorpixelid(1, 1, 13));
  ASSERT_EQ(6, testdetector.getdetectorpixelid(1, 1, 14));
  ASSERT_EQ(7, testdetector.getdetectorpixelid(1, 2, 13));
  ASSERT_EQ(8, testdetector.getdetectorpixelid(1, 2, 14));

  ASSERT_EQ(9, testdetector.getdetectorpixelid(1, 3, 11));
  ASSERT_EQ(10, testdetector.getdetectorpixelid(1, 3, 12));
  ASSERT_EQ(11, testdetector.getdetectorpixelid(1, 4, 11));
  ASSERT_EQ(12, testdetector.getdetectorpixelid(1, 4, 12));
  ASSERT_EQ(13, testdetector.getdetectorpixelid(1, 3, 9));
  ASSERT_EQ(14, testdetector.getdetectorpixelid(1, 3, 10));
  ASSERT_EQ(15, testdetector.getdetectorpixelid(1, 4, 9));
  ASSERT_EQ(16, testdetector.getdetectorpixelid(1, 4, 10));

  ASSERT_EQ(17, testdetector.getdetectorpixelid(1, 5, 7));
  ASSERT_EQ(18, testdetector.getdetectorpixelid(1, 5, 8));
  ASSERT_EQ(19, testdetector.getdetectorpixelid(1, 6, 7));
  ASSERT_EQ(20, testdetector.getdetectorpixelid(1, 6, 8));
  ASSERT_EQ(21, testdetector.getdetectorpixelid(1, 5, 5));
  ASSERT_EQ(22, testdetector.getdetectorpixelid(1, 5, 6));
  ASSERT_EQ(23, testdetector.getdetectorpixelid(1, 6, 5));
  ASSERT_EQ(24, testdetector.getdetectorpixelid(1, 6, 6));

  ASSERT_EQ(25, testdetector.getdetectorpixelid(1, 7, 3));
  ASSERT_EQ(26, testdetector.getdetectorpixelid(1, 7, 4));
  ASSERT_EQ(27, testdetector.getdetectorpixelid(1, 8, 3));
  ASSERT_EQ(28, testdetector.getdetectorpixelid(1, 8, 4));
  ASSERT_EQ(29, testdetector.getdetectorpixelid(1, 7, 1));
  ASSERT_EQ(30, testdetector.getdetectorpixelid(1, 7, 2));
  ASSERT_EQ(31, testdetector.getdetectorpixelid(1, 8, 1));
  ASSERT_EQ(32, testdetector.getdetectorpixelid(1, 8, 2));
}

TEST_F(MultiGridGeometryTest, TestDetectorIllegalWireAndGridid) {
  int panels = 1;
  int modules = 2;
  int grids = 48;
  int xwires = 4;
  int zwires = 16;
  MultiGridGeometry testdetector(panels, modules, grids, xwires, zwires);

  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 49, 65));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 96, 65));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 49, 128));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 96, 128));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 1, 1));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 48, 1));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 1, 64));
  ASSERT_EQ(-1, testdetector.getdetectorpixelid(1, 48, 64));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
