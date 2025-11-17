// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <cstdint>
#include <nmx/geometry/Config.h>
#include <nmx/geometry/NMXGeometry.h>

using namespace Nmx;

class NMXGeometryTest : public TestBase {
protected:
  Config TestConfig;
  Statistics Stats;
  NMXGeometry Geom{Stats, TestConfig, 4, 4};
  uint16_t VMM0{0};
  uint16_t VMM1{1};
  uint16_t VMM2{2};
  vmm3::VMM3Parser::VMM3Data readout{};

  void SetUp() override {
    // Initialize default hybrid configuration for tests
    TestConfig.NumHybrids = 1;
    TestConfig.Hybrids[0][0][0].Initialised = true;
    TestConfig.Hybrids[0][0][0].MinADC = 50;

    // Initialize default readout data structure
    readout.FiberId = 0; // Maps to Ring 0
    readout.FENId = 0;
    readout.VMM = 0; // HybridId = 0
    readout.Channel = 32;
    readout.OTADC = 100;
  }
  void TearDown() override {}
};

TEST_F(NMXGeometryTest, InvalidArguments) {
  ASSERT_EQ(Geom.coord(64, 1, 0, false), NMXGeometry::InvalidCoord);
  ASSERT_EQ(Geom.coord(63, 2, 0, false), NMXGeometry::InvalidCoord);
  ASSERT_NE(Geom.coord(63, 1, 0, false), NMXGeometry::InvalidCoord);
}

TEST_F(NMXGeometryTest, CoordinateCalculations) {
  // coord takes Channel, AsicId, Offset, ReversedChannels
  uint8_t AsicId = 0;
  uint16_t Offset = 0;
  bool ReversedChannels = false;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.coord(Channel, AsicId, Offset, ReversedChannels), Channel);
  }
  ReversedChannels = true;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.coord(Channel, AsicId, Offset, ReversedChannels),
              127 - Channel);
  }

  ReversedChannels = false;
  AsicId = 1;
  Offset = 512;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.coord(Channel, AsicId, Offset, ReversedChannels),
              Channel + 576);
  }
  ReversedChannels = true;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.coord(Channel, AsicId, Offset, ReversedChannels),
              575 - Channel);
  }
}

/// \brief Test ADC validation against minimum threshold
TEST_F(NMXGeometryTest, ADCValidation) {
  // Test with ADC below minimum
  readout.OTADC = 30; // Below threshold
  int64_t initialADCErrors = Geom.getNmxCounters().ADCErrors;
  bool validBelow = Geom.validateReadoutData(readout);
  ASSERT_FALSE(validBelow) << "ADC below minimum should fail validation";
  ASSERT_EQ(Geom.getNmxCounters().ADCErrors, initialADCErrors + 1)
      << "ADC error counter should increment by exactly 1 on validation "
         "failure";

  // Test with ADC exactly at minimum (boundary condition)
  readout.OTADC = 50; // At threshold
  initialADCErrors = Geom.getNmxCounters().ADCErrors;
  bool validAtMin = Geom.validateReadoutData(readout);
  ASSERT_TRUE(validAtMin) << "ADC at minimum threshold should pass validation";
  ASSERT_EQ(Geom.getNmxCounters().ADCErrors, initialADCErrors)
      << "Confirm no increment in ADC error counter on valid ADC at minimum "
         "threshold";

  // Test with ADC above minimum
  readout.OTADC = 100; // Above threshold
  bool validAbove = Geom.validateReadoutData(readout);
  ASSERT_TRUE(validAbove)
      << "ADC above minimum threshold should pass validation";
  ASSERT_EQ(Geom.getNmxCounters().ADCErrors, initialADCErrors)
      << "Counter should remain unchanged after valid ADC above minimum";
}

/// \brief Test channel validation within readout validation
TEST_F(NMXGeometryTest, ChannelValidationInReadout) {
  // Test with invalid channel (>63)
  readout.Channel = 64;
  int64_t initialChannelErrors = Geom.getNmxCounters().ChannelRangeErrors;
  bool validChannel64 = Geom.validateReadoutData(readout);
  ASSERT_FALSE(validChannel64)
      << "Channel 64 should fail validation (max is 63)";
  ASSERT_EQ(Geom.getNmxCounters().ChannelRangeErrors, initialChannelErrors + 1)
      << "Channel range error counter should increment by exactly 1";

  // Test with valid channel (63)
  readout.Channel = 63;
  initialChannelErrors = Geom.getNmxCounters().ChannelRangeErrors;
  bool validChannel63 = Geom.validateReadoutData(readout);
  ASSERT_TRUE(validChannel63) << "Channel 63 should pass validation";
  ASSERT_EQ(Geom.getNmxCounters().ChannelRangeErrors, initialChannelErrors)
      << "Counter should remain unchanged after valid Channel 63";

  // Test with valid channel (0)
  readout.Channel = 0;
  initialChannelErrors = Geom.getNmxCounters().ChannelRangeErrors;
  bool validChannel0 = Geom.validateReadoutData(readout);
  ASSERT_TRUE(validChannel0) << "Channel 0 should pass validation";
  ASSERT_EQ(Geom.getNmxCounters().ChannelRangeErrors, initialChannelErrors)
      << "Counter should remain unchanged after valid Channel 0";
}

/// \brief Test ASIC validation and AsicRangeErrors counter
TEST_F(NMXGeometryTest, AsicValidationInReadout) {
  // Test with valid ASIC IDs (0 and 1)
  // VMM0 has AsicId = 0, VMM1 has AsicId = 1
  readout.VMM = VMM0; // AsicId = 0
  int64_t initialAsicErrors = Geom.getNmxCounters().AsicRangeErrors;
  bool validAsic0 = Geom.validateReadoutData(readout);
  ASSERT_TRUE(validAsic0) << "AsicId 0 (VMM0) should pass validation";
  ASSERT_EQ(Geom.getNmxCounters().AsicRangeErrors, initialAsicErrors)
      << "Counter should remain unchanged after valid AsicId 0";

  readout.VMM = VMM1; // AsicId = 1
  initialAsicErrors = Geom.getNmxCounters().AsicRangeErrors;
  bool validAsic1 = Geom.validateReadoutData(readout);
  ASSERT_TRUE(validAsic1) << "AsicId 1 (VMM1) should pass validation";
  ASSERT_EQ(Geom.getNmxCounters().AsicRangeErrors, initialAsicErrors)
      << "Counter should remain unchanged after valid AsicId 1";

  // Test with invalid ASIC ID (2)
  readout.VMM = VMM2; // AsicId = 1 (even), but we'll test coord() directly
  initialAsicErrors = Geom.getNmxCounters().AsicRangeErrors;
  uint16_t result = Geom.coord(32, 2, 0, false); // AsicId = 2 (invalid)
  ASSERT_EQ(result, NMXGeometry::InvalidCoord)
      << "AsicId 2 should return InvalidCoord";
  ASSERT_EQ(Geom.getNmxCounters().AsicRangeErrors, initialAsicErrors + 1)
      << "AsicRangeErrors counter should increment by exactly 1 for invalid "
         "AsicId";

  // Test another invalid ASIC ID (3)
  initialAsicErrors = Geom.getNmxCounters().AsicRangeErrors;
  result = Geom.coord(16, 3, 0, false); // AsicId = 3 (invalid)
  ASSERT_EQ(result, NMXGeometry::InvalidCoord)
      << "AsicId 3 should return InvalidCoord";
  ASSERT_EQ(Geom.getNmxCounters().AsicRangeErrors, initialAsicErrors + 1)
      << "AsicRangeErrors counter should increment by exactly 1 for invalid "
         "AsicId 3";
}

/// \brief Test coordinate error counter when coord() is called with invalid
/// inputs
TEST_F(NMXGeometryTest, CoordinateErrorCounter) {
  // Test with invalid AsicId (>1)
  int64_t initialCoordErrors = Geom.getVmmCounters().CoordOverflow;
  uint16_t result = Geom.coord(32, 2, 0, false); // AsicId = 2 (invalid)
  ASSERT_EQ(result, NMXGeometry::InvalidCoord)
      << "Invalid AsicId should return InvalidCoord";
  ASSERT_EQ(Geom.getVmmCounters().CoordOverflow, initialCoordErrors + 1)
      << "Coordinate error counter should increment by exactly 1 on invalid "
         "AsicId";

  // Test with invalid Channel (>63)
  initialCoordErrors = Geom.getVmmCounters().CoordOverflow;
  result = Geom.coord(64, 0, 0, false); // Channel = 64 (invalid)
  ASSERT_EQ(result, NMXGeometry::InvalidCoord)
      << "Invalid Channel should return InvalidCoord";
  ASSERT_EQ(Geom.getVmmCounters().CoordOverflow, initialCoordErrors + 1)
      << "Coordinate error counter should increment by exactly 1 on invalid "
         "Channel";
}

/// \brief Test that readout validation encompasses all validation checks
TEST_F(NMXGeometryTest, ReadoutValidationIntegration) {
  // Use member readout from SetUp, modified for this test
  readout.VMM = 1; // AsicId = 1, HybridId = 0

  // This should pass all checks
  bool isValid = Geom.validateReadoutData(readout);
  ASSERT_TRUE(isValid) << "Valid readout should pass all validation checks";

  // Test invalid FEN
  readout.FENId = 10; // Invalid FEN (max is 4)
  int64_t initialFENErrors = Geom.getBaseCounters().FENErrors;
  isValid = Geom.validateReadoutData(readout);
  ASSERT_FALSE(isValid) << "Invalid FEN should fail validation";
  ASSERT_EQ(Geom.getBaseCounters().FENErrors, initialFENErrors + 1)
      << "FEN error counter should increment by exactly 1";
}

/// \brief Test ADC masking (10-bit) in readout validation
TEST_F(NMXGeometryTest, ADCMasking) {
  // Override the default MinADC with a high threshold to test masking
  TestConfig.Hybrids[0][0][0].MinADC = 500;

  // Set OTADC to 16-bit max with bits above 10 set
  // Only the lower 10 bits (0x3FF = 1023) should be considered
  readout.OTADC = 0xFFFF; // All 16 bits set, but only 10 bits used

  // 0xFFFF & 0x3FF = 0x3FF = 1023, which should be above 500 threshold
  int64_t initialADCErrors = Geom.getNmxCounters().ADCErrors;
  bool isValid = Geom.validateReadoutData(readout);
  ASSERT_TRUE(isValid)
      << "ADC value after masking to 10 bits should pass (1023 > 500)";
  ASSERT_EQ(Geom.getNmxCounters().ADCErrors, initialADCErrors)
      << "No ADC error should be recorded for valid masked ADC";

  // Now test with a value that fails after masking
  readout.OTADC = 0x0010; // Binary: 0000 0000 0001 0000, masked = 0x0010 = 16
  initialADCErrors = Geom.getNmxCounters().ADCErrors;
  isValid = Geom.validateReadoutData(readout);
  ASSERT_FALSE(isValid)
      << "Masked ADC value (16) should be below threshold (500)";
  ASSERT_EQ(Geom.getNmxCounters().ADCErrors, initialADCErrors + 1)
      << "ADC error counter should increment by exactly 1 for below-threshold "
         "masked value";
}

/// \brief Test Ring validation in readout validation
TEST_F(NMXGeometryTest, RingValidationInReadout) {
  // Modify readout to have invalid Ring
  readout.FiberId = 100; // Maps to Ring 50 (invalid, max is 4)

  int64_t initialRingErrors = Geom.getBaseCounters().RingErrors;
  bool isValid = Geom.validateReadoutData(readout);
  ASSERT_FALSE(isValid) << "Invalid Ring (from FiberId) should fail validation";
  ASSERT_EQ(Geom.getBaseCounters().RingErrors, initialRingErrors + 1)
      << "Ring error counter should increment by exactly 1";
}

/// \brief Test hybrid initialization check in readout validation
TEST_F(NMXGeometryTest, HybridNotInitializedValidation) {
  // Override to have no initialized hybrids - should fail validation
  readout.FiberId = 4;
  readout.FENId = 1;

  int64_t initialValidationErrors = Geom.getBaseCounters().ValidationErrors;
  int64_t initialHybridErrors = Geom.getVmmCounters().HybridMappingErrors;

  bool isValid = Geom.validateReadoutData(readout);
  ASSERT_FALSE(isValid) << "Uninitialized hybrid should fail validation";
  ASSERT_EQ(Geom.getBaseCounters().ValidationErrors,
            initialValidationErrors + 1)
      << "Validation error counter should increment by exactly 1 for "
         "uninitialized hybrid";
  ASSERT_EQ(Geom.getVmmCounters().HybridMappingErrors, initialHybridErrors + 1);
}

/// \brief Test calcPixel function with coordinate range validation against configured SizeX and SizeY
TEST_F(NMXGeometryTest, AIGEN_CalcPixelCoordinateRangeValidation) {
  // Create a custom config with larger coordinate ranges for testing
  Config LargeConfig;
  LargeConfig.NMXFileParameters.SizeX = 2000;  
  LargeConfig.NMXFileParameters.SizeY = 1500;
  
  // Create geometry with larger coordinate space
  Statistics LargeStats;
  NMXGeometry LargeGeom{LargeStats, LargeConfig, 4, 4};
  
  // Test 1: Valid event within configured coordinate range
  Event validEvent;
  Hit validHitX{1000, 1000, 100, 0};  // time=1000, coord=1000, weight=100, plane=0 (X)
  Hit validHitY{1000, 800, 100, 1};   // time=1000, coord=800, weight=100, plane=1 (Y)
  validEvent.insert(validHitX);
  validEvent.insert(validHitY);
  
  int64_t initialPixelErrors = LargeGeom.getBaseCounters().PixelErrors;
  uint32_t validPixelId = LargeGeom.calcPixel<Event>(validEvent);
  
  EXPECT_NE(validPixelId, 0) << "Event within coordinate range should produce valid pixel ID";
  EXPECT_EQ(LargeGeom.getBaseCounters().PixelErrors, initialPixelErrors)
      << "Valid coordinates should not increment pixel error counter";
  
  // Test 2: Invalid event - X coordinate exceeds SizeX
  // Let's debug what coordinates are actually being calculated
  Event invalidEventX;
  Hit invalidHitX{1000, 2500, 100, 0};  // time=1000, coord=2500, weight=100, plane=0 (X) 
  Hit validHitY2{1000, 500, 100, 1};    // time=1000, coord=500, weight=100, plane=1 (Y)
  invalidEventX.insert(invalidHitX);
  invalidEventX.insert(validHitY2);
  
  initialPixelErrors = LargeGeom.getBaseCounters().PixelErrors;
  uint32_t invalidPixelIdX = LargeGeom.calcPixel<Event>(invalidEventX);
  
  EXPECT_EQ(invalidPixelIdX, 0) << "Event with X coordinate > SizeX should return pixel ID 0";
  EXPECT_EQ(LargeGeom.getBaseCounters().PixelErrors, initialPixelErrors + 1)
      << "Invalid X coordinate should increment pixel error counter by 1";
  
  // Test 3: Invalid event - Y coordinate exceeds SizeY
  Event invalidEventY;
  Hit validHitX2{1000, 1200, 100, 0};   // time=1000, coord=1200, weight=100, plane=0 (X)
  Hit invalidHitY{1000, 1800, 100, 1};  // time=1000, coord=1800, weight=100, plane=1 (Y)
  invalidEventY.insert(validHitX2);
  invalidEventY.insert(invalidHitY);
  
  initialPixelErrors = LargeGeom.getBaseCounters().PixelErrors;
  uint32_t invalidPixelIdY = LargeGeom.calcPixel<Event>(invalidEventY);
  
  EXPECT_EQ(invalidPixelIdY, 0) << "Event with Y coordinate > SizeY should return pixel ID 0";
  EXPECT_EQ(LargeGeom.getBaseCounters().PixelErrors, initialPixelErrors + 1)
      << "Invalid Y coordinate should increment pixel error counter by 1";
  
  // Test 4: Boundary coordinates (exactly at SizeX-1 and SizeY-1)
  Event boundaryEvent;
  Hit boundaryHitX{1000, 1999, 100, 0};  // time=1000, coord=1999, weight=100, plane=0 (X)
  Hit boundaryHitY{1000, 1499, 100, 1};  // time=1000, coord=1499, weight=100, plane=1 (Y)
  boundaryEvent.insert(boundaryHitX);
  boundaryEvent.insert(boundaryHitY);
  
  initialPixelErrors = LargeGeom.getBaseCounters().PixelErrors;
  uint32_t boundaryPixelId = LargeGeom.calcPixel<Event>(boundaryEvent);
  
  EXPECT_NE(boundaryPixelId, 0) << "Event at coordinate boundaries should produce valid pixel ID";
  EXPECT_EQ(LargeGeom.getBaseCounters().PixelErrors, initialPixelErrors)
      << "Boundary coordinates should not increment pixel error counter";
}

/// \brief Test calcPixel function with small configured sizes to verify boundary checking
TEST_F(NMXGeometryTest, AIGEN_CalcPixelSmallCoordinateRanges) {
  // Create a config with small coordinate ranges
  Config SmallConfig;
  SmallConfig.NMXFileParameters.SizeX = 400;   // Small X range
  SmallConfig.NMXFileParameters.SizeY = 200;   // Small Y range
  
  Statistics SmallStats;
  NMXGeometry SmallGeom{SmallStats, SmallConfig, 4, 4};
  
  // Test 1: Valid event within small coordinate limits
  Event validEvent;
  Hit validHitX{1000, 200, 100, 0};  // time=1000, coord=200, weight=100, plane=0 (X)
  Hit validHitY{1000, 100, 100, 1};  // time=1000, coord=100, weight=100, plane=1 (Y)
  validEvent.insert(validHitX);
  validEvent.insert(validHitY);
  
  int64_t initialErrors = SmallGeom.getBaseCounters().PixelErrors;
  uint32_t validPixel = SmallGeom.calcPixel<Event>(validEvent);
  
  EXPECT_NE(validPixel, 0) << "Event within small coordinate limits should be valid";
  EXPECT_EQ(SmallGeom.getBaseCounters().PixelErrors, initialErrors)
      << "Valid event should not increment pixel error counter";
  
  // Test 2: Invalid event exceeding small SizeX
  Event invalidEventX;
  Hit invalidHitX{1000, 500, 100, 0};  // time=1000, coord=500, weight=100, plane=0 (X)
  Hit validHitY2{1000, 150, 100, 1};   // time=1000, coord=150, weight=100, plane=1 (Y)
  invalidEventX.insert(invalidHitX);
  invalidEventX.insert(validHitY2);
  
  initialErrors = SmallGeom.getBaseCounters().PixelErrors;
  uint32_t invalidPixelX = SmallGeom.calcPixel<Event>(invalidEventX);
  
  EXPECT_EQ(invalidPixelX, 0) << "Event beyond small SizeX should return pixel ID 0";
  EXPECT_EQ(SmallGeom.getBaseCounters().PixelErrors, initialErrors + 1)
      << "Invalid X coordinate should increment pixel error counter";
  
  // Test 3: Invalid event exceeding small SizeY
  Event invalidEventY;
  Hit validHitX2{1000, 300, 100, 0};   // time=1000, coord=300, weight=100, plane=0 (X)
  Hit invalidHitY{1000, 250, 100, 1};  // time=1000, coord=250, weight=100, plane=1 (Y)
  invalidEventY.insert(validHitX2);
  invalidEventY.insert(invalidHitY);
  
  initialErrors = SmallGeom.getBaseCounters().PixelErrors;
  uint32_t invalidPixelY = SmallGeom.calcPixel<Event>(invalidEventY);
  
  EXPECT_EQ(invalidPixelY, 0) << "Event beyond small SizeY should return pixel ID 0";
  EXPECT_EQ(SmallGeom.getBaseCounters().PixelErrors, initialErrors + 1)
      << "Invalid Y coordinate should increment pixel error counter";
  
  // Test 4: Test exact boundary values (SizeX-1, SizeY-1)
  Event boundaryEvent;
  Hit boundaryHitX{1000, 399, 100, 0};  // time=1000, coord=399, weight=100, plane=0 (X)
  Hit boundaryHitY{1000, 199, 100, 1};  // time=1000, coord=199, weight=100, plane=1 (Y)
  boundaryEvent.insert(boundaryHitX);
  boundaryEvent.insert(boundaryHitY);
  
  initialErrors = SmallGeom.getBaseCounters().PixelErrors;
  uint32_t boundaryPixel = SmallGeom.calcPixel<Event>(boundaryEvent);
  
  EXPECT_NE(boundaryPixel, 0) << "Event at exact boundaries should be valid";
  EXPECT_EQ(SmallGeom.getBaseCounters().PixelErrors, initialErrors)
      << "Boundary coordinates should not increment error counter";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
