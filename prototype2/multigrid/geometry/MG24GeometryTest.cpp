/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/TSCTimer.h>
#include <multigrid/geometry/MG24Geometry.h>
#include <test/TestBase.h>

class MG24DetectorTest : public TestBase {
protected:
  Multigrid::ModuleLogicalGeometry geom;
  void SetUp() override {
  }
  void TearDown()  override {
  }
};

/** Test cases below */

TEST_F(MG24DetectorTest, IsWireIsGrid) {
  Multigrid::MG24GeometryA mgdet;
  mgdet.max_channel(128);
  geom.max_grid(mgdet.max_channel() - mgdet.max_wire());

  for (int i = 0; i <= 79; i++) {
    EXPECT_TRUE(mgdet.isWire(0, i));
    EXPECT_FALSE(mgdet.isGrid(0, i));
  }

  for (int i = 80; i <= 127; i++) {
    EXPECT_FALSE(mgdet.isWire(0, i)) << " bad wire eval at " << i;
    EXPECT_TRUE(mgdet.isGrid(0, i)) << " bad wire eval at " << i;
  }

  EXPECT_FALSE(mgdet.isWire(0, 128));
  EXPECT_FALSE(mgdet.isGrid(0, 128));
}

// \todo these tests are more confusing than the implementation being tested
TEST_F(MG24DetectorTest, XZCoordinatesVariantA) {
  Multigrid::MG24GeometryA mgdet;
  mgdet.max_channel(128);
  geom.max_grid(mgdet.max_channel() - mgdet.max_wire());

  for (int xoffset = 0; xoffset < 4; xoffset++) {
    MESSAGE() << "Lower wires: " << xoffset * 16
              << " to " << (xoffset * 16 + 15) << "\n";
    for (int zoffset = 0; zoffset < 16; zoffset++) {
      int channel = xoffset * 16 + zoffset;
      EXPECT_EQ(xoffset, geom.x_from_wire(mgdet.wire(0, channel)))
              << " bad eval xof=" << xoffset << " zof="
              << zoffset << " chan=" << channel;
      EXPECT_EQ(zoffset, geom.z_from_wire(mgdet.wire(0, channel)))
              << " bad eval xof=" << xoffset << " zof="
              << zoffset << " chan=" << channel;
    }
  }

  for (int xoffset = 0; xoffset < 4; xoffset++) {
    MESSAGE() << "Upper wires: " << 64 + xoffset * 4 << " to " << (64 + xoffset * 4 + 3) << "\n";
    for (int zoffset = 0; zoffset < 4; zoffset++) {
      int channel = 64 + xoffset * 4 + zoffset;
      //MESSAGE() << "channel: " << channel << "\n";
      EXPECT_EQ(xoffset, geom.x_from_wire(mgdet.wire(0, channel)));
      EXPECT_EQ(16 + zoffset, geom.z_from_wire(mgdet.wire(0, channel)));
    }
  }
}

TEST_F(MG24DetectorTest, YCoordinatesVariantA) {
  Multigrid::MG24GeometryA mgdet;
  mgdet.max_channel(127);
  geom.max_grid(mgdet.max_channel() - mgdet.max_wire());

  for (int channel = 80; channel < 127; channel++) {
    EXPECT_EQ(channel - 80 , geom.y_from_grid(mgdet.grid(0, channel)));
  }
}

// This is disabled by default

//TEST_F(MG24DetectorTest, ManualInspectionVariantA) {
//  Multigrid::MG24GeometryA mgdet;
//  mgdet.max_channel(128);
//
//  for (uint16_t channel = 0; channel <= 127; channel++) {
//    if (mgdet.isGrid(0, channel))
//      MESSAGE() << "chan=" << channel << "  ->  grid:"
//                << mgdet.grid(0, channel)
//                << " y:" << mgdet.y_from_grid(mgdet.grid(0, channel))
//                << "\n";
//    else if (mgdet.isWire(0, channel))
//      MESSAGE() << "chan=" << channel << "  ->  wire:"
//                << mgdet.wire(0, channel)
//                << " x:" << mgdet.x_from_wire(mgdet.wire(0, channel))
//                << " z:" << mgdet.z_from_wire(mgdet.wire(0, channel))
//                << "\n";
//    else
//      MESSAGE() << "chan=" << channel << "  ->  ERROR\n";
//  }
//}

// \todo tests for VariantB

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
