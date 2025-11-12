// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <freia/geometry/FreiaGeometry.h>

using namespace Freia;
using namespace vmm3;

class FreiaGeometryTest : public TestBase {
protected:
  Statistics Stats;
  Config Conf; // default instrument config
  FreiaGeometry Geom{Stats, Conf};
  vmm3::VMM3Parser::VMM3Data TestData{}; // Member variable for test data
  uint16_t Cassette0Xoffset{0};
  uint16_t Cassette0Yoffset{0};
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
    TestData.VMM = VMMX;       // VMM=1 (odd, X plane for Freia), HybridId = 1 >> 1 = 0
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

TEST_F(FreiaGeometryTest, Coordinates) {
  // Test X coordinates (VMM1 handles X plane for Freia)
  for (unsigned int channel = 0; channel < 64; channel++) {
    auto result = Geom.calculateCoordinate(Cassette0Xoffset, 0, VMMX, channel);
    ASSERT_EQ(result.coord, 63 - channel);
    ASSERT_TRUE(result.isXPlane);
  }
  
  // Test Y coordinates (VMM0 handles Y plane for Freia)  
  for (unsigned int channel = 16; channel < 48; channel++) {
    auto result = Geom.calculateCoordinate(0, Cassette0Yoffset, VMMY, channel);
    ASSERT_EQ(result.coord, 47 - channel);
    ASSERT_FALSE(result.isXPlane);
  }
}

TEST_F(FreiaGeometryTest, XCoordErrors) {
  // Test bad Channel for X coordinate (VMM1 uses strips, max channel 63)
  TestData.Channel = 64;     // Invalid channel for strips (too high)
  ASSERT_FALSE(Geom.validateReadoutData(TestData));
  EXPECT_EQ(Geom.getVmmCounters().StripChannelRangeErrors, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);
}

TEST_F(FreiaGeometryTest, YCoordErrors) {
  // Test bad channels for Y coordinate (VMM0 uses wires, valid range 16-47)
  TestData.VMM = VMMY;       // VMM=0 (even, Y plane)
  
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

TEST_F(FreiaGeometryTest, BoundaryTests) {
  // Test very large VMM values (should still work due to bit masking in isXCoord)
  auto result1 = Geom.calculateCoordinate(0, 0, -1, 32); // VMM 255 is odd, should be X plane for Freia
  ASSERT_TRUE(result1.isXPlane);
  // Channel 32 is valid for X plane strips, so should calculate: 63 - 32 = 31
  ASSERT_EQ(result1.coord, 31);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);
  
  auto result2 = Geom.calculateCoordinate(0, 0, -2, 32); // VMM 254 is even, should be Y plane for Freia
  ASSERT_FALSE(result2.isXPlane);
  // Channel 32 is valid for Y plane wires (range 16-47), should calculate: 47 - 32 = 15
  ASSERT_EQ(result2.coord, 15);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);
  
  // Test very large channel values using validateReadoutData
  TestData.VMM = VMMX;
  TestData.Channel = 255; // Channel 255 is way out of range for strips
  ASSERT_FALSE(Geom.validateReadoutData(TestData));
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 0);
  EXPECT_EQ(Geom.getVmmCounters().StripChannelRangeErrors, 1);
  
  TestData.VMM = VMMY;
  TestData.Channel = 255; // Channel 255 is way out of range for wires (16-47)
  ASSERT_FALSE(Geom.validateReadoutData(TestData));
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().WireChannelRangeErrors, 1);
  
  // Test very large offsets (should not cause overflow issues)
  TestData.VMM = VMMX;
  TestData.Channel = 32;
  auto result5 = Geom.calculateCoordinate(65000, 0, VMMX, 32);
  ASSERT_TRUE(result5.isXPlane);
  // X coordinate calculation: XOffset + (63 - 32) = 65000 + 31 = 65031
  // This should fit in uint16_t (max 65535)
  ASSERT_EQ(result5.coord, 65031);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 1); // Still 1 from the channel validation above
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  
  // Test offset that WILL cause overflow
  TestData.VMM = VMMX;
  TestData.Channel = 0;
  auto result6 = Geom.calculateCoordinate(-10, 0, VMMX, 0);
  ASSERT_TRUE(result6.isXPlane);
  // X coordinate calculation would be: XOffset + (63 - 0) = 65526 + 63 = 65589
  // This overflows uint16_t, so should return InvalidCoord
  ASSERT_EQ(result6.coord, Geom.InvalidCoord);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 2); // Incremented due to overflow
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  EXPECT_EQ(Geom.getVmmCounters().CoordOverflow, 1);
  
  // Test Y coordinate overflow (Freia uses wires for Y)
  TestData.VMM = VMMY;
  TestData.Channel = 47;
  auto result7 = Geom.calculateCoordinate(0, -10, VMMY, 47);
  ASSERT_FALSE(result7.isXPlane);  
  // Y coordinate calculation would be: YOffset + (47 - 47) = 65526 + 0 = 65526
  // This should still be valid (less than 65535)
  ASSERT_EQ(result7.coord, 65526);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 1);
  
  // Test Y coordinate that will overflow
  TestData.VMM = VMMY;
  TestData.Channel = 16;
  auto result8 = Geom.calculateCoordinate(0, -10, VMMY, 16);
  ASSERT_FALSE(result8.isXPlane);
  // Y coordinate calculation would be: YOffset + (47 - 16) = 65526 + 31 = 65557
  // This overflows uint16_t, so should return InvalidCoord
  ASSERT_EQ(result8.coord, Geom.InvalidCoord);
  EXPECT_EQ(Geom.getVmmCounters().InvalidYCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().InvalidXCoord, 2);
  EXPECT_EQ(Geom.getVmmCounters().CoordOverflow, 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
