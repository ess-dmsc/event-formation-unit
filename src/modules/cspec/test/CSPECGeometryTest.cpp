// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <cspec/geometry/CSPECGeometry.h>

using namespace Cspec;

class CSPECGeometryTest : public TestBase {
protected:
  CSPECGeometry Geom;
  uint16_t VMM0{0};
  uint16_t VMM1{1};
  uint16_t VMM2{2};

  void SetUp() override {}
  void TearDown() override {}
};

// Should match the ICD
TEST_F(CSPECGeometryTest, DefaultCSPEC) {
  ASSERT_TRUE(Geom.isWire(VMM0));
  ASSERT_TRUE(Geom.isGrid(VMM1));
  ASSERT_TRUE(Geom.isGrid(VMM2));
}

TEST_F(CSPECGeometryTest, XAndZCoordinateCalculations) {
  // xAndzCoord takes FENID, HybridID, VMMID, Channel, XOffset, Rotated

  // Vessel 0, very first xAndzCoord position
  ASSERT_EQ(Geom.xAndzCoord(0, 0, 0, 0, 32, 0, false), 0);
  // Vessel 14, position is mid-column
  ASSERT_EQ(Geom.xAndzCoord(0, 0, 0, 1, 43, 168, false), 2763);
  // Vessel 25, rotated, column 0 and column 1
  ASSERT_EQ(Geom.xAndzCoord(0, 0, 0, 1, 43, 300, true), 4923);
  ASSERT_EQ(Geom.xAndzCoord(0, 1, 0, 1, 43, 300, true), 4827);
  // Vessel 34, final pixel
  ASSERT_EQ(Geom.xAndzCoord(0, 1, 0, 1, 63, 372, false), 6143);

  int RingID = 0;
  int FENID = 0;
  int HybridID = 0;
  int VMMID = 0;
  int XOffset = 0;
  bool Rotated = false;

  for (int Channel = 32; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.xAndzCoord(RingID, FENID, HybridID, VMMID, Channel, XOffset,
                              Rotated),
              Channel - 32);
  }
  VMMID = 1;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.xAndzCoord(RingID, FENID, HybridID, VMMID, Channel, XOffset,
                              Rotated),
              Channel + 32);
  }
  FENID = 1;
  VMMID = 0;
  for (int Channel = 32; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.xAndzCoord(RingID, FENID, HybridID, VMMID, Channel, XOffset,
                              Rotated),
              Channel + 64);
  }
  VMMID = 1;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.xAndzCoord(RingID, FENID, HybridID, VMMID, Channel, XOffset,
                              Rotated),
              Channel + 128);
  }
}

TEST_F(CSPECGeometryTest, InvalidXAndZCoordinates) {
  // invalid HybridID
  ASSERT_EQ(Geometry::InvalidCoord, 65535); // Invalid Coordinate == 65535

  ASSERT_EQ(Geom.xAndzCoord(0, 0, 1, 0, 0, 0, false), Geometry::InvalidCoord);
  ASSERT_EQ(Geom.xAndzCoord(0, 0, 1, 0, 0, 0, false), Geometry::InvalidCoord);
  // invalid channel on vmm0
  ASSERT_EQ(Geom.xAndzCoord(0, 0, 0, 0, 10, 0, false), Geometry::InvalidCoord);

  // invalid VMM
  ASSERT_EQ(Geom.xAndzCoord(0, 0, 0, 4, 0, 0, false), Geometry::InvalidCoord);
}

TEST_F(CSPECGeometryTest, YCoordinateCalculations) {
  // yCoord = VMM & Channel specific value

  // Full length vessel, grid 0 and grid 139
  ASSERT_EQ(Geom.yCoord(1, 0, 58, 0, false, false), 0);
  ASSERT_EQ(Geom.yCoord(2, 1, 5, 0, false, false), 139);

  // short rotated vessel
  ASSERT_EQ(Geom.yCoord(1, 0, 40, 50, true, true), 10);

  // short not rotated vessel
  ASSERT_EQ(Geom.yCoord(1, 0, 40, 89, false, true), 129);

  int HybridID = 1;
  int VMMID = 0;
  int YOffset = 0;
  bool Rotated = false;
  bool Short = false;

  for (int Channel = 58; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.yCoord(HybridID, VMMID, Channel, YOffset, Rotated, Short),
              Channel - 58);
  }
  VMMID = 1;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.yCoord(HybridID, VMMID, Channel, YOffset, Rotated, Short),
              Channel + 6);
  }
  HybridID = 2;
  VMMID = 0;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.yCoord(HybridID, VMMID, Channel, YOffset, Rotated, Short),
              Channel + 70);
  }
  VMMID = 1;
  for (int Channel = 0; Channel < 6; ++Channel) {
    ASSERT_EQ(Geom.yCoord(HybridID, VMMID, Channel, YOffset, Rotated, Short),
              Channel + 134);
  }
}

TEST_F(CSPECGeometryTest, InvalidYCoordinates) {
  // invalid channel on short vessel
  ASSERT_EQ(Geom.yCoord(1, 0, 60, 0, false, true), Geometry::InvalidCoord);

  // invalid channel on hybrid 1 vmm0
  ASSERT_EQ(Geom.xAndzCoord(0, 1, 0, 10, 0, false, false),
            Geometry::InvalidCoord);

  // invalid channel on hybrid 2 vmm 1
  ASSERT_EQ(Geom.xAndzCoord(0, 2, 1, 10, 0, false, false),
            Geometry::InvalidCoord);

  // invalid hybrid
  ASSERT_EQ(Geom.xAndzCoord(0, 3, 0, 0, 0, false, false),
            Geometry::InvalidCoord);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
