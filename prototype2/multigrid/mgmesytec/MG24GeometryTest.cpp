/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <libs/include/TSCTimer.h>
#include <multigrid/mgmesytec/MG24Geometry.h>
#include <test/TestBase.h>

class MG24DetectorTest : public TestBase {};

/** Test cases below */

TEST_F(MG24DetectorTest, IsWireIsGrid) {
  MG25Geometry mgdet;

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

#if 0
TEST_F(MG24DetectorTest, XZCoordinates) {
  MG24Detector mgdet;
  int digitizer = 0;
  for (int xoffset = 0; xoffset < 4; xoffset++) {
    MESSAGE() << "Lower wires: " << xoffset * 16 << " to " << (xoffset * 16 + 15) << "\n";
    for (int zoffset = 0; zoffset < 16; zoffset++) {
      int channel = xoffset * 16 + zoffset;
      ASSERT_EQ(xoffset, mgdet.xcoord(digitizer, channel));
      ASSERT_EQ(-1, mgdet.ycoord(channel));
      ASSERT_EQ(zoffset , mgdet.zcoord(channel));
    }
  }

  for (int xoffset = 0; xoffset < 4; xoffset++) {
    MESSAGE() << "Upper wires: " << 64 + xoffset * 4 << " to " << (64 + xoffset * 4 + 3) << "\n";
    for (int zoffset = 0; zoffset < 4; zoffset++) {
      int channel = 64 + xoffset * 4 + zoffset;
      //MESSAGE() << "channel: " << channel << "\n";
      ASSERT_EQ(xoffset, mgdet.xcoord(digitizer, channel));
      ASSERT_EQ(-1 , mgdet.ycoord(channel));
      ASSERT_EQ(16 + zoffset, mgdet.zcoord(channel));
    }
  }
}

TEST_F(MG24DetectorTest, YCoordinates) {
  MG24Detector mgdet;
  int digitizer = 0;
  for (int channel = 80; channel < 127; channel++) {
    ASSERT_EQ(-1, mgdet.xcoord(digitizer, channel));
    ASSERT_EQ(channel - 80 , mgdet.ycoord(channel));
    ASSERT_EQ(-1, mgdet.zcoord(channel));
  }
}

#endif

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
