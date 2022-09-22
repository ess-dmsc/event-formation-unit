// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <dream/geometry/CDTGeometry.h>

using namespace Dream;

class CDTGeometryTest : public TestBase {
protected:
  CDTGeometry geometry;
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(CDTGeometryTest, PixelOffsets) {
  ASSERT_EQ(geometry.getPixelOffset(Config::FwEndCap), 0);
  ASSERT_EQ(geometry.getPixelOffset(Config::BwEndCap), 71680);
  ASSERT_EQ(geometry.getPixelOffset(Config::Mantle), 229376);
  ASSERT_EQ(geometry.getPixelOffset(Config::SANS), 720896);
  ASSERT_EQ(geometry.getPixelOffset(Config::HR), 1122304);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/*
TEST_F(CDTGeometryTest, Setup) {
  ASSERT_EQ(SUMO6.Cassettes, 10);
  ASSERT_EQ(SUMO5.Cassettes, 8);
  ASSERT_EQ(SUMO4.Cassettes, 6);
  ASSERT_EQ(SUMO3.Cassettes, 4);
  ASSERT_EQ(Cuboid.Cassettes, 8);
  ASSERT_EQ(Mantle.Cassettes, 6);
}

TEST_F(CDTGeometryTest, GetXSUMOS) {
  //                   Sect, Cass, Ctr
  ASSERT_EQ(SUMO6.getXSUMO(0, 0, 0), 0);
  ASSERT_EQ(SUMO6.getXSUMO(0, 9, 1), 19);
  ASSERT_EQ(SUMO5.getXSUMO(0, 0, 0), 20);
  ASSERT_EQ(SUMO5.getXSUMO(0, 7, 1), 35);
  ASSERT_EQ(SUMO4.getXSUMO(0, 0, 0), 36);
  ASSERT_EQ(SUMO4.getXSUMO(0, 5, 1), 47);
  ASSERT_EQ(SUMO3.getXSUMO(0, 0, 0), 48);
  ASSERT_EQ(SUMO3.getXSUMO(0, 3, 1), 55);
}

TEST_F(CDTGeometryTest, GetYSUMOS) {
  int FrontStrip{0};
  ASSERT_EQ(SUMO6.getYSUMO(TopWire, FrontStrip), 0);
  ASSERT_EQ(SUMO6.getYSUMO(BottomWire, FrontStrip), 15);
  ASSERT_EQ(SUMO5.getYSUMO(TopWire, FrontStrip), 0);
  ASSERT_EQ(SUMO5.getYSUMO(BottomWire, FrontStrip), 15);
  ASSERT_EQ(SUMO4.getYSUMO(TopWire, FrontStrip), 0);
  ASSERT_EQ(SUMO4.getYSUMO(BottomWire, FrontStrip), 15);
  ASSERT_EQ(SUMO3.getYSUMO(TopWire, FrontStrip), 0);
  ASSERT_EQ(SUMO3.getYSUMO(BottomWire, FrontStrip), 15);
}

TEST_F(CDTGeometryTest, LogicalEndcap) {
  int FrontStrip{0};
  int BackStrip{15};
  // Top left
  ASSERT_EQ(SUMO6.getXSUMO(0, 0, 0), 0);
  ASSERT_EQ(SUMO6.getYSUMO(TopWire, FrontStrip), 0);

  // Top right
  ASSERT_EQ(SUMO3.getXSUMO(22, 3, 1), 1287);
  ASSERT_EQ(SUMO3.getYSUMO(TopWire, FrontStrip), 0);

  // Bottom left
  ASSERT_EQ(SUMO6.getXSUMO(0, 0, 0), 0);
  ASSERT_EQ(SUMO6.getYSUMO(BottomWire, BackStrip), 255);

  // Bottom right
  ASSERT_EQ(SUMO3.getXSUMO(22, 3, 1), 1287);
  ASSERT_EQ(SUMO3.getYSUMO(BottomWire, BackStrip), 255);
}

// Testing that x-coordinates rotate correctly (first and last Cuboid)
TEST_F(CDTGeometryTest, CuboidXRotate) {
  int Index{0};
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, TopWire, 0), 32);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, TopWire, 1), 47);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, TopWire, 2), 47);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, TopWire, 3), 32);

  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, TopWire, 0), 47);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, TopWire, 1), 47);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, TopWire, 2), 32);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, TopWire, 3), 32);

  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, BottomWire, 0), 47);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, BottomWire, 1), 32);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, BottomWire, 2), 32);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, BottomWire, 3), 47);

  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, BottomWire, 0), 32);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, BottomWire, 1), 32);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, BottomWire, 2), 47);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, BottomWire, 3), 47);

  Index = 32;
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, TopWire, 0), 64);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, TopWire, 1), 79);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, TopWire, 2), 79);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, TopWire, 3), 64);

  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, TopWire, 0), 79);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, TopWire, 1), 79);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, TopWire, 2), 64);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, TopWire, 3), 64);

  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, BottomWire, 0), 79);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, BottomWire, 1), 64);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, BottomWire, 2), 64);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass7, Ctr1, BottomWire, 3), 79);

  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, BottomWire, 0), 64);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, BottomWire, 1), 64);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, BottomWire, 2), 79);
  ASSERT_EQ(Cuboid.getXCuboid(Index, Cass0, Ctr0, BottomWire, 3), 79);
}

// Testing that y-coordinates rotate correctly (first and last Cuboid)
TEST_F(CDTGeometryTest, CuboidYRotate) {
  int FrontStrip{0};
  int Index{0};
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass0, Ctr0, TopWire, FrontStrip, 0), 0);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass0, Ctr0, TopWire, FrontStrip, 1), 0);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass0, Ctr0, TopWire, FrontStrip, 2), 15);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass0, Ctr0, TopWire, FrontStrip, 3), 15);

  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass7, Ctr1, TopWire, FrontStrip, 0), 0);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass7, Ctr1, TopWire, FrontStrip, 1), 15);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass7, Ctr1, TopWire, FrontStrip, 2), 15);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass7, Ctr1, TopWire, FrontStrip, 3), 0);

  Index = 32;
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass0, Ctr0, TopWire, FrontStrip, 0), 96);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass0, Ctr0, TopWire, FrontStrip, 1), 96);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass0, Ctr0, TopWire, FrontStrip, 2), 111);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass0, Ctr0, TopWire, FrontStrip, 3), 111);

  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass7, Ctr1, TopWire, FrontStrip, 0), 96);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass7, Ctr1, TopWire, FrontStrip, 1), 111);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass7, Ctr1, TopWire, FrontStrip, 2), 111);
  ASSERT_EQ(Cuboid.getYCuboid(Index, Cass7, Ctr1, TopWire, FrontStrip, 3), 96);
}

// Testing 'corners' in each layer (Cuboids 0, 2, 31 and 32)
TEST_F(CDTGeometryTest, LogicalSANS) {
  int Rot{0}; // canonical

  // Move through increasing depth (z)
  for (int Strip = 0; Strip < 16; Strip++) {
    int Y = Strip * 7 * 16;
    // Top left
    ASSERT_EQ(Cuboid.getXCuboid(0, 0, 0, TopWire, Rot), 32);
    ASSERT_EQ(Cuboid.getYCuboid(0, 0, 0, TopWire, Strip, Rot), Y + 0);

    // Top right
    ASSERT_EQ(Cuboid.getXCuboid(2, 7, 1, TopWire, Rot), 79);
    ASSERT_EQ(Cuboid.getYCuboid(2, 7, 1, TopWire, Strip, Rot), Y + 0);

    // Bottom left
    ASSERT_EQ(Cuboid.getXCuboid(31, 0, 0, BottomWire, Rot), 32);
    ASSERT_EQ(Cuboid.getYCuboid(31, 0, 0, BottomWire, Strip, Rot), Y + 111);

    // Bottom right
    ASSERT_EQ(Cuboid.getXCuboid(32, 7, 1, TopWire, Rot), 79);
    ASSERT_EQ(Cuboid.getYCuboid(32, 7, 1, BottomWire, Strip, Rot), Y + 111);
  }
}
*/
