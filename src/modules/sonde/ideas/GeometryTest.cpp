/** Copyright (C) 2017 European Spallation Source ERIC */

#include <algorithm>
#include <memory>
#include <sonde/Geometry.h>
#include <sonde/ideas/Data.h>
#include <sonde/ideas/TestData.h>
#include <common/testutils/TestBase.h>

using namespace Sonde;

class IDEASGeometryTest : public TestBase {
protected:
  Geometry geometry;
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(IDEASGeometryTest, MaxPixelID) {
  int res = geometry.getmaxpixelid();
  ASSERT_EQ(res, 64);
}

TEST_F(IDEASGeometryTest, InvalidAsic) {
  ASSERT_NE(-1, geometry.getdetectorpixelid(0, 0x00));
  ASSERT_NE(-1, geometry.getdetectorpixelid(0, 0x40));
  ASSERT_NE(-1, geometry.getdetectorpixelid(0, 0x80));
  ASSERT_NE(-1, geometry.getdetectorpixelid(0, 0xc0));

  ASSERT_EQ(-1, geometry.getdetectorpixelid(0, 0x100));
  ASSERT_EQ(-1, geometry.getdetectorpixelid(0, 0x140));
  ASSERT_EQ(-1, geometry.getdetectorpixelid(0, 0x180));
  ASSERT_EQ(-1, geometry.getdetectorpixelid(0, 0x1c0));
}

TEST_F(IDEASGeometryTest, InvalidChannel) {
  for (int i = 0; i < 15; i++) {
    int res = geometry.getdetectorpixelid(0, i);
    ASSERT_NE(res, -1);
  }
  for (int i = 16; i < 63; i++) {
    int res = geometry.getdetectorpixelid(0, i);
    ASSERT_EQ(res, -1);
  }
}

int asch(int asic, int channel) { return (asic << 6) + channel; }

TEST_F(IDEASGeometryTest, ValidateCornersASIC_0_and_1) {
  // First row
  ASSERT_EQ(1, geometry.getdetectorpixelid(0, asch(1, 0)));
  ASSERT_EQ(2, geometry.getdetectorpixelid(0, asch(1, 1)));
  ASSERT_EQ(3, geometry.getdetectorpixelid(0, asch(1, 2)));
  ASSERT_EQ(4, geometry.getdetectorpixelid(0, asch(1, 3)));
  ASSERT_EQ(5, geometry.getdetectorpixelid(0, asch(0, 15)));
  ASSERT_EQ(6, geometry.getdetectorpixelid(0, asch(0, 14)));
  ASSERT_EQ(7, geometry.getdetectorpixelid(0, asch(0, 13)));
  ASSERT_EQ(8, geometry.getdetectorpixelid(0, asch(0, 12)));
  // Second row
  ASSERT_EQ(9, geometry.getdetectorpixelid(0, asch(1, 4)));
  ASSERT_EQ(10, geometry.getdetectorpixelid(0, asch(1, 5)));
  ASSERT_EQ(11, geometry.getdetectorpixelid(0, asch(1, 6)));
  ASSERT_EQ(12, geometry.getdetectorpixelid(0, asch(1, 7)));
  ASSERT_EQ(13, geometry.getdetectorpixelid(0, asch(0, 11)));
  ASSERT_EQ(14, geometry.getdetectorpixelid(0, asch(0, 10)));
  ASSERT_EQ(15, geometry.getdetectorpixelid(0, asch(0, 9)));
  ASSERT_EQ(16, geometry.getdetectorpixelid(0, asch(0, 8)));
  // Third row
  ASSERT_EQ(17, geometry.getdetectorpixelid(0, asch(1, 8)));
  ASSERT_EQ(18, geometry.getdetectorpixelid(0, asch(1, 9)));
  ASSERT_EQ(19, geometry.getdetectorpixelid(0, asch(1, 10)));
  ASSERT_EQ(20, geometry.getdetectorpixelid(0, asch(1, 11)));
  ASSERT_EQ(21, geometry.getdetectorpixelid(0, asch(0, 7)));
  ASSERT_EQ(22, geometry.getdetectorpixelid(0, asch(0, 6)));
  ASSERT_EQ(23, geometry.getdetectorpixelid(0, asch(0, 5)));
  ASSERT_EQ(24, geometry.getdetectorpixelid(0, asch(0, 4)));

  // Fourth row
  ASSERT_EQ(25, geometry.getdetectorpixelid(0, asch(1, 12)));
  ASSERT_EQ(26, geometry.getdetectorpixelid(0, asch(1, 13)));
  ASSERT_EQ(27, geometry.getdetectorpixelid(0, asch(1, 14)));
  ASSERT_EQ(28, geometry.getdetectorpixelid(0, asch(1, 15)));
  ASSERT_EQ(29, geometry.getdetectorpixelid(0, asch(0, 3)));
  ASSERT_EQ(30, geometry.getdetectorpixelid(0, asch(0, 2)));
  ASSERT_EQ(31, geometry.getdetectorpixelid(0, asch(0, 1)));
  ASSERT_EQ(32, geometry.getdetectorpixelid(0, asch(0, 0)));
}

TEST_F(IDEASGeometryTest, ValidateCornersASIC_2_and_3) {
  // Fifth row
  ASSERT_EQ(33, geometry.getdetectorpixelid(0, asch(3, 0)));
  ASSERT_EQ(34, geometry.getdetectorpixelid(0, asch(3, 1)));
  ASSERT_EQ(35, geometry.getdetectorpixelid(0, asch(3, 2)));
  ASSERT_EQ(36, geometry.getdetectorpixelid(0, asch(3, 3)));
  ASSERT_EQ(37, geometry.getdetectorpixelid(0, asch(2, 15)));
  ASSERT_EQ(38, geometry.getdetectorpixelid(0, asch(2, 14)));
  ASSERT_EQ(39, geometry.getdetectorpixelid(0, asch(2, 13)));
  ASSERT_EQ(40, geometry.getdetectorpixelid(0, asch(2, 12)));
  // Sixth row
  ASSERT_EQ(41, geometry.getdetectorpixelid(0, asch(3, 4)));
  ASSERT_EQ(42, geometry.getdetectorpixelid(0, asch(3, 5)));
  ASSERT_EQ(43, geometry.getdetectorpixelid(0, asch(3, 6)));
  ASSERT_EQ(44, geometry.getdetectorpixelid(0, asch(3, 7)));
  ASSERT_EQ(45, geometry.getdetectorpixelid(0, asch(2, 11)));
  ASSERT_EQ(46, geometry.getdetectorpixelid(0, asch(2, 10)));
  ASSERT_EQ(47, geometry.getdetectorpixelid(0, asch(2, 9)));
  ASSERT_EQ(48, geometry.getdetectorpixelid(0, asch(2, 8)));
  // Seventh row
  ASSERT_EQ(49, geometry.getdetectorpixelid(0, asch(3, 8)));
  ASSERT_EQ(50, geometry.getdetectorpixelid(0, asch(3, 9)));
  ASSERT_EQ(51, geometry.getdetectorpixelid(0, asch(3, 10)));
  ASSERT_EQ(52, geometry.getdetectorpixelid(0, asch(3, 11)));
  ASSERT_EQ(53, geometry.getdetectorpixelid(0, asch(2, 7)));
  ASSERT_EQ(54, geometry.getdetectorpixelid(0, asch(2, 6)));
  ASSERT_EQ(55, geometry.getdetectorpixelid(0, asch(2, 5)));
  ASSERT_EQ(56, geometry.getdetectorpixelid(0, asch(2, 4)));

  // Eights row
  ASSERT_EQ(57, geometry.getdetectorpixelid(0, asch(3, 12)));
  ASSERT_EQ(58, geometry.getdetectorpixelid(0, asch(3, 13)));
  ASSERT_EQ(59, geometry.getdetectorpixelid(0, asch(3, 14)));
  ASSERT_EQ(60, geometry.getdetectorpixelid(0, asch(3, 15)));
  ASSERT_EQ(61, geometry.getdetectorpixelid(0, asch(2, 3)));
  ASSERT_EQ(62, geometry.getdetectorpixelid(0, asch(2, 2)));
  ASSERT_EQ(63, geometry.getdetectorpixelid(0, asch(2, 1)));
  ASSERT_EQ(64, geometry.getdetectorpixelid(0, asch(2, 0)));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
