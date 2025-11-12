// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <freia/geometry/EstiaGeometry.h>
#include <gtest/gtest.h>

using namespace Freia;
using namespace vmm3;

class EstiaGeometryTest : public TestBase {
protected:
  Statistics Stats;
  Config Conf; // default instrument config
  EstiaGeometry Geom{Stats, Conf};
  vmm3::VMM3Parser::VMM3Data TestData{}; // Member variable for test data
  uint16_t Cassette0XOffset{0};
  uint16_t Cassette0YOffset{0};
  uint16_t VMMX{1};
  uint16_t VMMY{0};
  
  void SetUp() override {
    // Initialize the hybrid for Ring 0, FEN 0 so validateHybrid passes
    // HybridId = VMM >> 1 = 1 >> 1 = 0 for VMMX, 0 >> 1 = 0 for VMMY
    auto &Hybrid = Conf.getHybrid(0, 0, 0);
    Hybrid.Initialised = true;
    Hybrid.HybridId = "TestHybrid";
    Hybrid.HybridNumber = 0;
    Hybrid.XOffset = 0;
    Hybrid.YOffset = 0;
    
    // Initialize base TestData with values that pass all validations except channel
    TestData.FiberId = 0;      // Ring = calcRing(0) = 0 (valid, < MaxRing=10)
    TestData.FENId = 0;        // Valid FEN (< MaxFEN=13)
    TestData.VMM = VMMX;       // VMM=1 (odd, X plane), HybridId = 1 >> 1 = 0
    TestData.Channel = 32;     // Valid channel for wires (range 16-47)
    TestData.DataLength = 0;
    TestData.TimeHigh = 0;
    TestData.TimeLow = 0;
    TestData.BC = 0;
    TestData.OTADC = 0;
    TestData.GEO = 0;
    TestData.TDC = 0;
  }
  
  void TearDown() override {}
};

TEST_F(EstiaGeometryTest, Coordinates) {
  // Test X coordinates (VMM1 handles X plane for Estia, uses wires)
  for (unsigned int Channel = 16; Channel < 48; Channel++) {
    unsigned int xCoord = Channel - 16; // Channel offset for wires
    auto result = Geom.calculateCoordinate(Cassette0XOffset, 0, VMMX, Channel);
    ASSERT_TRUE(result.isXPlane);
    ASSERT_EQ(result.coord, xCoord);
  }

  // Test Y coordinates (VMM0 handles Y plane for Estia, uses strips)
  for (unsigned int Channel = 0; Channel < 64; Channel++) {
    unsigned int yCoord = 63 - Channel;
    auto result = Geom.calculateCoordinate(0, Cassette0YOffset, VMMY, Channel);
    ASSERT_FALSE(result.isXPlane);
    ASSERT_EQ(result.coord, yCoord);
  }
}

TEST_F(EstiaGeometryTest, XCoordErrors) {
  // Test invalid channels for X coordinate (VMM1 uses wires, valid range 16-47)
  // All other validations (Ring, FEN, Hybrid) should pass, only channel should fail
  
  TestData.Channel = 15;     // Invalid channel for wires (too low)
  ASSERT_FALSE(Geom.validateReadoutData(TestData)); // bad Channel (too low)
  EXPECT_EQ(Geom.getVmmCounters().WireChannelRangeErrors, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);

  TestData.Channel = 48;     // Invalid channel for wires (too high)
  ASSERT_FALSE(Geom.validateReadoutData(TestData)); // bad Channel (too high)
  EXPECT_EQ(Geom.getVmmCounters().WireChannelRangeErrors, 2);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);
}

TEST_F(EstiaGeometryTest, YCoordErrors) {
  // Test invalid channels for Y coordinate (VMM0 uses strips, valid range 0-63)
  TestData.VMM = VMMY;       // VMM=0 (even, Y plane)
  
  TestData.Channel = 64;     // Invalid channel for strips (too high)
  ASSERT_FALSE(Geom.validateReadoutData(TestData)); // bad Channel (too high)
  EXPECT_EQ(Geom.getVmmCounters().StripChannelRangeErrors, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
}

TEST_F(EstiaGeometryTest, BoundaryTests) {
  // Test very large VMM values (should still work due to bit masking in
  // isXCoord)
  auto result1 = Geom.calculateCoordinate(
      0, 0, -1, 32); // VMM 255 is odd, should be X plane
  ASSERT_TRUE(result1.isXPlane);
  // Channel 32 is valid for X plane wires (range 16-47), so should calculate:
  // 32 - 16 = 16
  ASSERT_EQ(result1.coord, 16);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);

  auto result2 = Geom.calculateCoordinate(
      0, 0, -2, 32); // VMM 254 is even, should be Y plane
  ASSERT_FALSE(result2.isXPlane);
  // Channel 32 is valid for Y plane strips, should calculate: 63 - 32 = 31
  ASSERT_EQ(result2.coord, 31);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);

  // Test very large channel values using validateReadoutData
  TestData.VMM = VMMY;
  TestData.Channel = 255;
  ASSERT_FALSE(Geom.validateReadoutData(TestData)); // Channel 255 is way out of range for strips
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().StripChannelRangeErrors, 1);

  TestData.VMM = VMMX;
  TestData.Channel = 255;
  ASSERT_FALSE(Geom.validateReadoutData(TestData)); // Channel 255 is way out of range for wires (16-47)
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().WireChannelRangeErrors, 1);

  // Test very large offsets (should not cause overflow issues)
  auto result5 = Geom.calculateCoordinate(-35, 65000, VMMY, 32);
  ASSERT_FALSE(result5.isXPlane);
  // Y coordinate calculation: YOffset + (63 - 32) = 65000 + 31 = 65031
  // This should fit in uint16_t (max 65535)
  ASSERT_EQ(result5.coord, 65031);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);

  // Test offset that WILL cause overflow
  auto result6 = Geom.calculateCoordinate(0, -10, VMMY, 0);
  ASSERT_FALSE(result6.isXPlane);
  // Y coordinate calculation would be: YOffset + (63 - 0) = 65500 + 63 = 65563
  // This overflows uint16_t, so should return InvalidCoord
  ASSERT_EQ(result6.coord, Geom.InvalidCoord);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().CoordOverflow, 1);

  // Test X coordinate overflow (Estia uses different wire calculation)
  auto result7 = Geom.calculateCoordinate(-10, 0, VMMX, 47);
  ASSERT_TRUE(result7.isXPlane);
  // X coordinate calculation would be: XOffset + (47 - 16) = 65520 + 31 = 65551
  // This overflows uint16_t, so should return InvalidCoord
  ASSERT_EQ(result7.coord, Geom.InvalidCoord);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().CoordOverflow, 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
