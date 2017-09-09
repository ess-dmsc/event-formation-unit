/** Copyright (C) 2017 European Spallation Source ERIC */



#include <algorithm>
#include <memory>
#include <sonde/Geometry.h>
#include <sonde/ideas/Data.h>
#include <sonde/ideas/TestData.h>
#include <test/TestBase.h>

using namespace std;

class IDEASGeometryTest : public TestBase {
protected:
  SoNDeGeometry geometry;
  virtual void SetUp() { }
  virtual void TearDown() { }
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

TEST_F(IDEASGeometryTest, ValidateCorners) {
    // First row
    ASSERT_EQ(1, geometry.getdetectorpixelid(0, 0));
    ASSERT_EQ(4, geometry.getdetectorpixelid(0, 3));
    ASSERT_EQ(5, geometry.getdetectorpixelid(0, 0x40));
    ASSERT_EQ(8, geometry.getdetectorpixelid(0, 0x43));
    // Second row
    ASSERT_EQ(9, geometry.getdetectorpixelid(0, 0x04));
    ASSERT_EQ(12, geometry.getdetectorpixelid(0, 0x07));
    ASSERT_EQ(13, geometry.getdetectorpixelid(0, 0x44));
    ASSERT_EQ(16, geometry.getdetectorpixelid(0, 0x47));
    // Fifth row
    ASSERT_EQ(33, geometry.getdetectorpixelid(0, 0x80));
    ASSERT_EQ(36, geometry.getdetectorpixelid(0, 0x83));
    ASSERT_EQ(37, geometry.getdetectorpixelid(0, 0xc0));
    ASSERT_EQ(40, geometry.getdetectorpixelid(0, 0xc3));

    // Eigth row
    ASSERT_EQ(57, geometry.getdetectorpixelid(0, 0x8c));
    ASSERT_EQ(60, geometry.getdetectorpixelid(0, 0x8f));
    ASSERT_EQ(61, geometry.getdetectorpixelid(0, 0xcc));
    ASSERT_EQ(64, geometry.getdetectorpixelid(0, 0xcf));
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
