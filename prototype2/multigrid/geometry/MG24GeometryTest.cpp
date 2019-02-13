/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <libs/include/TSCTimer.h>
#include <multigrid/geometry/MG24Geometry.h>
#include <test/TestBase.h>

class MG24DetectorTest : public TestBase {};

/** Test cases below */

TEST_F(MG24DetectorTest, IsWireIsGrid) {
  Multigrid::MG24Geometry mgdet;
  mgdet.max_channel(128);

  for (int i = 0; i <= 79; i++) {
    EXPECT_TRUE(mgdet.isWire(i));
    EXPECT_FALSE(mgdet.isGrid(i));
  }

  for (int i = 80; i <= 127; i++) {
    EXPECT_FALSE(mgdet.isWire(i)) << " bad wire eval at " << i;
    EXPECT_TRUE(mgdet.isGrid(i)) << " bad wire eval at " << i;
  }

  EXPECT_FALSE(mgdet.isWire(128));
  EXPECT_FALSE(mgdet.isGrid(128));
}

// \todo these tests are more confusing than the implementation being tested

TEST_F(MG24DetectorTest, XZCoordinatesVariantA) {
  Multigrid::MG24GeometryA mgdet;
  mgdet.max_channel(128);
  mgdet.max_z(16);
  mgdet.swap_wires(false);

  for (int xoffset = 0; xoffset < 4; xoffset++) {
    MESSAGE() << "Lower wires: " << xoffset * 16
              << " to " << (xoffset * 16 + 15) << "\n";
    for (int zoffset = 0; zoffset < 16; zoffset++) {
      int channel = xoffset * 16 + zoffset;
      EXPECT_EQ(xoffset, mgdet.x(channel))
              << " bad eval xof=" << xoffset << " zof="
              << zoffset << " chan=" << channel;
      EXPECT_EQ(zoffset, mgdet.z(channel))
              << " bad eval xof=" << xoffset << " zof="
              << zoffset << " chan=" << channel;
    }
  }

  for (int xoffset = 0; xoffset < 4; xoffset++) {
    MESSAGE() << "Upper wires: " << 64 + xoffset * 4 << " to " << (64 + xoffset * 4 + 3) << "\n";
    for (int zoffset = 0; zoffset < 4; zoffset++) {
      int channel = 64 + xoffset * 4 + zoffset;
      //MESSAGE() << "channel: " << channel << "\n";
      EXPECT_EQ(xoffset, mgdet.x(channel));
      EXPECT_EQ(16 + zoffset, mgdet.z(channel));
    }
  }
}

TEST_F(MG24DetectorTest, YCoordinatesVariantA) {
  Multigrid::MG24GeometryA mgdet;
  mgdet.max_channel(127);

  for (int channel = 80; channel < 127; channel++) {
    EXPECT_EQ(channel - 80 , mgdet.y(channel));
  }
}

// \todo tests for VariantB

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
