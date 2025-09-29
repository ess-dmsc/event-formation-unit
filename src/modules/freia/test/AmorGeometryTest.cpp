// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <geometry/Config.h>
#include <common/testutils/TestBase.h>
#include <freia/geometry/AmorGeometry.h>

using namespace Freia;

class AmorGeometryTest : public TestBase {
protected:
  Statistics Stats;
  Config Conf; // default instrument config
  AmorGeometry Geom{Stats, Conf};
  uint16_t Cassette0Xoffset{0};
  uint16_t Cassette0Yoffset{0};
  uint16_t VMMX{0};
  uint16_t VMMY{1};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(AmorGeometryTest, Coordinates) {
  // Test X coordinates (VMM0 handles X plane for Amor - opposite of Freia)
  for (unsigned int i = 0; i < 64; i++) {
    auto result = Geom.calculateCoordinate(Cassette0Xoffset, 0, VMMX, i);
    ASSERT_TRUE(result.isXPlane);
    ASSERT_EQ(result.coord, 63 - i);
  }

  uint YCoordMinChannel = 16;
  uint YCoordMaxChannel = 47;
  uint MaxYOffset = 1024;
  uint YOffsetJumps = 32;

  // Test Y coordinates (VMM1 handles Y plane for Amor)
  for (unsigned int i = YCoordMinChannel; i < YCoordMaxChannel; i++) {
    for (unsigned int YOffset = 0; YOffset < MaxYOffset;
         YOffset += YOffsetJumps) {
      auto result = Geom.calculateCoordinate(0, YOffset, VMMY, i);
      ASSERT_FALSE(result.isXPlane);
      ASSERT_EQ(result.coord, YCoordMaxChannel - i + YOffset);
    }
  }
}

TEST_F(AmorGeometryTest, XCoordErrors) {
  // Test bad Channel for X coordinate (VMM0 uses strips, max channel 63)
  ASSERT_FALSE(Geom.validateChannel(VMMX, 64));
  EXPECT_EQ(Geom.getVmmGeometryCounters().StripChannelRangeErrors, 1);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 0);
}

TEST_F(AmorGeometryTest, YCoordErrors) {
  // Test bad channels for Y coordinate (VMM1 uses wires, valid range 16-47)
  ASSERT_FALSE(Geom.validateChannel(VMMY, 15));
  EXPECT_EQ(Geom.getVmmGeometryCounters().WireChannelRangeErrors, 1);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 1);
  
  ASSERT_FALSE(Geom.validateChannel(VMMY, 48));
  EXPECT_EQ(Geom.getVmmGeometryCounters().WireChannelRangeErrors, 2);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 2);
}

TEST_F(AmorGeometryTest, BoundaryTests) {
  // Test very large VMM values (should still work due to bit masking in isXCoord)
  auto result1 = Geom.calculateCoordinate(0, 0, -2, 32); // VMM 254 is even, should be X plane for Amor
  ASSERT_TRUE(result1.isXPlane);
  // Channel 32 is valid for X plane strips, so should calculate: 63 - 32 = 31
  ASSERT_EQ(result1.coord, 31);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 0);
  
  auto result2 = Geom.calculateCoordinate(0, 0, -1, 32); // VMM 255 is odd, should be Y plane for Amor
  ASSERT_FALSE(result2.isXPlane);
  // Channel 32 is valid for Y plane wires (range 16-47), should calculate: 47 - 32 = 15
  ASSERT_EQ(result2.coord, 15);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 0);
  
  // Test very large channel values using validateChannel
  ASSERT_FALSE(Geom.validateChannel(VMMX, 255)); // Channel 255 is way out of range for strips
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 0);
  EXPECT_EQ(Geom.getVmmGeometryCounters().StripChannelRangeErrors, 1);
  
  ASSERT_FALSE(Geom.validateChannel(VMMY, 255)); // Channel 255 is way out of range for wires (16-47)
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 1);
  EXPECT_EQ(Geom.getVmmGeometryCounters().WireChannelRangeErrors, 1);
  
  // Test very large offsets (should not cause overflow issues)
  auto result5 = Geom.calculateCoordinate(65000, 0, VMMX, 32);
  ASSERT_TRUE(result5.isXPlane);
  // X coordinate calculation: XOffset + (63 - 32) = 65000 + 31 = 65031
  // This should fit in uint16_t (max 65535)
  ASSERT_EQ(result5.coord, 65031);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 1);
  
  // Test offset that WILL cause overflow
  auto result6 = Geom.calculateCoordinate(-10, 0, VMMX, 0);
  ASSERT_TRUE(result6.isXPlane);
  // X coordinate calculation would be: XOffset + (63 - 0) = 65526 + 63 = 65589
  // This overflows uint16_t, so should return InvalidCoord
  ASSERT_EQ(result6.coord, Geom.InvalidCoord);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 2);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 1);
  EXPECT_EQ(Geom.getVmmGeometryCounters().CoordOverflow, 1);
  
  // Test Y coordinate overflow (Amor uses wires for Y)
  auto result7 = Geom.calculateCoordinate(0, -10, VMMY, 47);
  ASSERT_FALSE(result7.isXPlane);  
  // Y coordinate calculation would be: YOffset + (47 - 47) = 65526 + 0 = 65526
  // This should still be valid (less than 65535)
  ASSERT_EQ(result7.coord, 65526);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 2);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 1);
  
  // Test Y coordinate that will overflow
  auto result8 = Geom.calculateCoordinate(0, -10, VMMY, 16);
  ASSERT_FALSE(result8.isXPlane);
  // Y coordinate calculation would be: YOffset + (47 - 16) = 65526 + 31 = 65557
  // This overflows uint16_t, so should return InvalidCoord
  ASSERT_EQ(result8.coord, Geom.InvalidCoord);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidYCoord, 2);
  EXPECT_EQ(Geom.getVmmGeometryCounters().InvalidXCoord, 2);
  EXPECT_EQ(Geom.getVmmGeometryCounters().CoordOverflow, 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
