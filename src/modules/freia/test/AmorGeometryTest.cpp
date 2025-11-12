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
using namespace vmm3;

class AmorGeometryTest : public TestBase {
protected:
  Statistics Stats;
  Config Conf; // default instrument config (no loading/applying needed)
  AmorGeometry Geom{Stats, Conf}; // Member geometry instance
  vmm3::VMM3Parser::VMM3Data TestData{}; // Member variable for test data
  uint16_t Cassette0Xoffset{0};
  uint16_t Cassette0Yoffset{0};
  uint16_t VMMX{0};
  uint16_t VMMY{1};
  
  void SetUp() override {
    // Initialize the hybrid for Ring 2, FEN 0, HybridId 0 so validateHybrid passes
    // HybridId = VMM >> 1 = 0 >> 1 = 0 for VMMX, 1 >> 1 = 0 for VMMY
    auto &Hybrid = Conf.getHybrid(2, 0, 0);
    Hybrid.Initialised = true;
    Hybrid.HybridId = "TestHybrid";
    Hybrid.HybridNumber = 0;
    Hybrid.XOffset = 0;
    Hybrid.YOffset = 0;
    
    // Initialize base TestData with values that pass all validations except channel
    TestData.FiberId = 4;      // Ring = calcRing(4) = 2 (valid, < MaxRing=10)
    TestData.FENId = 0;        // Valid FEN (< MaxFEN=13)
    TestData.VMM = VMMX;       // VMM=0 (even, X plane for AMOR), HybridId = 0 >> 1 = 0
    TestData.Channel = 32;     // Valid channel for strips (range 0-63)
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
  // All other validations (Ring, FEN, Hybrid) should pass, only channel should fail
  
  TestData.Channel = 64;     // Invalid channel for strips (too high)
  ASSERT_FALSE(Geom.validateReadoutData(TestData));
  EXPECT_EQ(Geom.getVmmCounters().StripChannelRangeErrors, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);
}

TEST_F(AmorGeometryTest, YCoordErrors) {
  // Test bad channels for Y coordinate (VMM1 uses wires, valid range 16-47)
  TestData.VMM = VMMY;       // VMM=1 (odd, Y plane for AMOR)
  
  TestData.Channel = 15;     // Invalid channel for wires (too low)
  ASSERT_FALSE(Geom.validateReadoutData(TestData));
  EXPECT_EQ(Geom.getVmmCounters().WireChannelRangeErrors, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  
  TestData.Channel = 48;     // Invalid channel for wires (too high)
  ASSERT_FALSE(Geom.validateReadoutData(TestData));
  EXPECT_EQ(Geom.getVmmCounters().WireChannelRangeErrors, 2);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 2);
}

TEST_F(AmorGeometryTest, TestValidateReadoutDataPass) {
  /// Test validateReadoutData with valid readout data
  /// This tests the AMOR-specific validation logic
  VMM3Parser::VMM3Data ValidData;
  ValidData.FiberId = 4;      // Ring 2 (4 / 2 = 2)
  ValidData.FENId = 0;        // Valid FEN
  ValidData.VMM = 0;          // VMM 0, HybridId 0 (even, X plane)
  ValidData.Channel = 32;     // Valid channel (0-63 for strips)
  
  // Capture initial counters state
  auto initialVmmCounters = Geom.getVmmCounters();
  auto initialBaseCounters = Geom.getBaseCounters();
  
  // Validate the readout data - should return true for valid data
  bool isValid = Geom.validateReadoutData(ValidData);
  ASSERT_TRUE(isValid);
  
  // Verify no error counters were incremented
  auto finalVmmCounters = Geom.getVmmCounters();
  auto finalBaseCounters = Geom.getBaseCounters();
  EXPECT_EQ(finalBaseCounters.RingErrors, initialBaseCounters.RingErrors);
  EXPECT_EQ(finalBaseCounters.FENErrors, initialBaseCounters.FENErrors);
  EXPECT_EQ(finalVmmCounters.HybridMappingErrors, initialVmmCounters.HybridMappingErrors);
  EXPECT_EQ(finalVmmCounters.InvalidXCoord, initialVmmCounters.InvalidXCoord);
  EXPECT_EQ(finalVmmCounters.InvalidYCoord, initialVmmCounters.InvalidYCoord);
}

TEST_F(AmorGeometryTest, TestValidateReadoutDataFailOnRing) {
  /// Test validateReadoutData with invalid ring number
  /// This tests that the AMOR validation catches out-of-range ring values
  VMM3Parser::VMM3Data InvalidRingData;
  InvalidRingData.FiberId = 200;  // Ring 100 (200 / 2 = 100) - way out of range
  InvalidRingData.FENId = 0;
  InvalidRingData.VMM = 0;
  InvalidRingData.Channel = 32;
  
  auto initialBaseCounters = Geom.getBaseCounters();
  
  // Validate the readout data - should return false for invalid ring
  bool isValid = Geom.validateReadoutData(InvalidRingData);
  ASSERT_FALSE(isValid);
  
  // Verify that RingErrors counter was incremented
  auto finalBaseCounters = Geom.getBaseCounters();
  EXPECT_GT(finalBaseCounters.RingErrors, initialBaseCounters.RingErrors);
}

TEST_F(AmorGeometryTest, TestValidateReadoutDataFailOnChannelk) {
  /// Test validateReadoutData with invalid channel number
  /// This tests that the AMOR validation catches invalid channel values
  VMM3Parser::VMM3Data InvalidChannelData;
  InvalidChannelData.FiberId = 4;   // Ring 2 (valid)
  InvalidChannelData.FENId = 0;     // FEN 0 (valid)
  InvalidChannelData.VMM = 0;       // VMM 0 (even, so X plane for AMOR)
  InvalidChannelData.Channel = 100; // Invalid channel (> 63 for X plane)
  
  auto initialVmmCounters = Geom.getVmmCounters();
  
  // Validate the readout data - should return false for invalid channel
  bool isValid = Geom.validateReadoutData(InvalidChannelData);
  ASSERT_FALSE(isValid);
  
  // Verify that StripChannelRangeErrors was incremented (X plane uses strips)
  auto finalVmmCounters = Geom.getVmmCounters();
  EXPECT_GT(finalVmmCounters.StripChannelRangeErrors, initialVmmCounters.StripChannelRangeErrors);
}

TEST_F(AmorGeometryTest, BoundaryTests) {
  // Test very large VMM values (should still work due to bit masking in isXCoord)
  auto result1 = Geom.calculateCoordinate(0, 0, -2, 32); // VMM 254 is even, should be X plane for Amor
  ASSERT_TRUE(result1.isXPlane);
  // Channel 32 is valid for X plane strips, so should calculate: 63 - 32 = 31
  ASSERT_EQ(result1.coord, 31);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);
  
  auto result2 = Geom.calculateCoordinate(0, 0, -1, 32); // VMM 255 is odd, should be Y plane for Amor
  ASSERT_FALSE(result2.isXPlane);
  // Channel 32 is valid for Y plane wires (range 16-47), should calculate: 47 - 32 = 15
  ASSERT_EQ(result2.coord, 15);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);
  
  // Test very large channel values using validateReadoutData
  TestData.VMM = VMMX;
  TestData.Channel = 255;
  ASSERT_FALSE(Geom.validateReadoutData(TestData)); // Channel 255 is way out of range for strips
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().StripChannelRangeErrors, 1);
  
  TestData.VMM = VMMY;
  TestData.Channel = 255;
  ASSERT_FALSE(Geom.validateReadoutData(TestData)); // Channel 255 is way out of range for wires (16-47)
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().WireChannelRangeErrors, 1);
  
  // Test very large offsets (should not cause overflow issues)
  auto result5 = Geom.calculateCoordinate(65000, 0, VMMX, 32);
  ASSERT_TRUE(result5.isXPlane);
  // X coordinate calculation: XOffset + (63 - 32) = 65000 + 31 = 65031
  // This should fit in uint16_t (max 65535)
  ASSERT_EQ(result5.coord, 65031);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  
  // Test offset that WILL cause overflow
  auto result6 = Geom.calculateCoordinate(-10, 0, VMMX, 0);
  ASSERT_TRUE(result6.isXPlane);
  // X coordinate calculation would be: XOffset + (63 - 0) = 65526 + 63 = 65589
  // This overflows uint16_t, so should return InvalidCoord
  ASSERT_EQ(result6.coord, VMM3Geometry::InvalidCoord);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().CoordOverflow, 1);
  
  // Test Y coordinate overflow (Amor uses wires for Y)
  auto result7 = Geom.calculateCoordinate(0, -10, VMMY, 47);
  ASSERT_FALSE(result7.isXPlane);  
  // Y coordinate calculation would be: YOffset + (47 - 47) = 65526 + 0 = 65526
  // This should still be valid (less than 65535)
  ASSERT_EQ(result7.coord, 65526);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  
  // Test Y coordinate that will overflow
  auto result8 = Geom.calculateCoordinate(0, -10, VMMY, 16);
  ASSERT_FALSE(result8.isXPlane);
  // Y coordinate calculation would be: YOffset + (47 - 16) = 65526 + 31 = 65557
  // This overflows uint16_t, so should return InvalidCoord
  ASSERT_EQ(result8.coord, VMM3Geometry::InvalidCoord);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().CoordOverflow, 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
