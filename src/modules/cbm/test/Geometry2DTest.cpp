// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for CBM geometry validation and pixel calculations
///
//===----------------------------------------------------------------------===//

#include "CbmTypes.h"
#include <common/testutils/TestBase.h>
#include <gtest/gtest.h>
#include <memory>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/geometry/Geometry0D.h>
#include <modules/cbm/geometry/Geometry2D.h>
#include <modules/cbm/readout/Parser.h>

using namespace cbm;

class Geometry2DTest : public TestBase {
protected:
  Statistics Stats;
  Config CbmConfig;
  std::unique_ptr<Geometry2D> geom;

  void SetUp() override {

    CbmConfig.CbmParms.MonitorRing = 11; // Default monitor ring
    CbmConfig.CbmParms.MaxRing = 11;     // Default max ring
    CbmConfig.CbmParms.MaxFENId = 23;    // Default max FEN

    // Create topology configuration with null pointers - tests don't use
    // topology data
    CbmConfig.TopologyMapPtr = std::make_unique<HashMap2D<Topology>>(24);
    std::unique_ptr<Topology> nullTopo = nullptr;
    CbmConfig.TopologyMapPtr->add(3, 0, nullTopo);

    // Create geometry with 512x512 dimensions, like in production config
    geom =
        std::make_unique<Geometry2D>(Stats, CbmConfig, "test_source", 512, 512);
  }

  void TearDown() override {}
};

TEST_F(Geometry2DTest, Constructor) {
  EXPECT_NE(geom, nullptr);
  EXPECT_EQ(geom->getWidth(), 512);
  EXPECT_EQ(geom->getHeight(), 512);
  EXPECT_EQ(geom->getSourceName(), "test_source");
}

TEST_F(Geometry2DTest, ValidateValidReadoutData) {
  Parser::CbmReadout readout{};

  // Set valid Ring and FEN
  readout.FiberId = 22; // Ring = 22/2 = 11, valid (MaxRing = 11)
  readout.FENId = 3;    // Valid topology (configured in SetUp)
  readout.Channel = 0;  // Valid topology (configured in SetUp)

  // Test various valid coordinate combinations
  readout.Pos.XPos = 0;
  readout.Pos.YPos = 0;
  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 0);
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(geom->getGeometryCounters().XPosErrors, 0);
  EXPECT_EQ(geom->getGeometryCounters().YPosErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);

  readout.Pos.XPos = 256;
  readout.Pos.YPos = 256;
  EXPECT_TRUE(geom->validateReadoutData(readout));

  readout.Pos.XPos = 511;
  readout.Pos.YPos = 511;
  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);

  readout.Pos.XPos = 100;
  readout.Pos.YPos = 200;
  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors,
            0); // Verify no errors were counted
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 0);
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(geom->getGeometryCounters().XPosErrors, 0);
  EXPECT_EQ(geom->getGeometryCounters().YPosErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);
}

TEST_F(Geometry2DTest, ValidateTopology) {
  Parser::CbmReadout readout{};

  readout.Pos.XPos = 100; // Valid coordinates
  readout.Pos.YPos = 100;

  readout.FiberId = 22; // Valid Ring

  // Valid topology
  readout.FENId = 3;   // Valid topology (configured in SetUp)
  readout.Channel = 0; // Valid topology (configured in SetUp)
  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);

  // Invalid topology - FEN/Channel not in configuration
  readout.FENId = 5; // This FEN/Channel combo not configured
  readout.Channel = 5;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 1);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 0);

  // Another invalid topology
  readout.FENId = 10;
  readout.Channel = 0; // FEN 10 exists but Channel 0 not configured for it
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 2);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 2);
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 0);
}

TEST_F(Geometry2DTest, ValidateRingAndFEN) {
  Parser::CbmReadout readout{};
  readout.FENId = 3;   // Valid topology (configured in SetUp)
  readout.Channel = 0; // Valid topology (configured in SetUp)

  readout.Pos.XPos = 100; // Valid coordinates
  readout.Pos.YPos = 100;

  // Valid Ring
  readout.FiberId = 22; // Ring = 22/2 = 11
  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);

  // Invalid Ring (FiberId = 24 -> Ring = 12, MaxRing = 11)
  readout.FiberId = 24;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);

  // Invalid FEN (FENId = 24, MaxFEN = 23) - use valid Ring that matches
  // MonitorRing
  readout.FiberId = 22; // Ring = 11, matches MonitorRing
  readout.FENId = 24;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 1); // No change
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 1);  // FEN validator reached
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 2);

  // Both Ring and FEN invalid - only Ring counted due to short-circuit
  readout.FiberId = 30; // Ring = 15, exceeds MaxRing
  readout.FENId = 30;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 2);
  EXPECT_EQ(geom->getBaseCounters().FENErrors,
            1); // FEN validator not reached (Ring fails first)
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 3);
}

TEST_F(Geometry2DTest, ValidateCoordinates) {
  Parser::CbmReadout readout{};
  // Use valid Ring and FEN with topology for all coordinate tests
  readout.FiberId = 22;
  readout.FENId = 3;   // Valid topology (configured in SetUp)
  readout.Channel = 0; // Valid topology (configured in SetUp)

  // Test X validator fails with valid Y
  readout.Pos.XPos = 512;
  readout.Pos.YPos = 100; // Valid
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getGeometryCounters().XPosErrors, 1);
  EXPECT_EQ(geom->getGeometryCounters().YPosErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 0);

  readout.Pos.XPos = 3840;
  readout.Pos.YPos = 200; // Valid
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getGeometryCounters().XPosErrors, 2);
  EXPECT_EQ(geom->getGeometryCounters().YPosErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 2);

  // Test Y validator fails with valid X
  readout.Pos.XPos = 100; // Valid
  readout.Pos.YPos = 512;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getGeometryCounters().XPosErrors, 2);
  EXPECT_EQ(geom->getGeometryCounters().YPosErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 3);

  readout.Pos.XPos = 200; // Valid
  readout.Pos.YPos = 4027;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getGeometryCounters().XPosErrors, 2);
  EXPECT_EQ(geom->getGeometryCounters().YPosErrors, 2);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 4);

  // Both X and Y invalid - only X is counted due to short-circuit
  readout.Pos.XPos = 65535;
  readout.Pos.YPos = 65535;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getGeometryCounters().XPosErrors, 3);
  EXPECT_EQ(geom->getGeometryCounters().YPosErrors,
            2); // Y validator not reached
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 5);

  // No pixel errors - calcPixel hasn't been called
  EXPECT_EQ(geom->getBaseCounters().PixelErrors, 0);
}

TEST_F(Geometry2DTest, CalcPixelErrorCounting) {
  Parser::CbmReadout readout{};

  // When calcPixel is called with invalid coordinates (out of ESSGeometry
  // range), ESSGeometry returns 0, which triggers PixelErrors counter increment
  readout.Pos.XPos = 512;
  readout.Pos.YPos = 512;
  EXPECT_EQ(geom->calcPixel(readout), 0);
  EXPECT_EQ(geom->getBaseCounters().PixelErrors, 1);

  // Another out-of-range pixel calculation
  readout.Pos.XPos = 3840;
  readout.Pos.YPos = 4027;
  EXPECT_EQ(geom->calcPixel(readout), 0);
  EXPECT_EQ(geom->getBaseCounters().PixelErrors, 2);

  // Note: validateReadoutData is independent and not called by calcPixel,
  // so coordinate-specific errors are not incremented here
  EXPECT_EQ(geom->getGeometryCounters().XPosErrors, 0);
  EXPECT_EQ(geom->getGeometryCounters().YPosErrors, 0);
}

TEST_F(Geometry2DTest, CalcPixelValid) {
  Parser::CbmReadout readout{};

  // Test corner pixels
  readout.Pos.XPos = 0;
  readout.Pos.YPos = 0;
  EXPECT_EQ(geom->calcPixel(readout), 1); // pixel2D adds 1

  readout.Pos.XPos = 511;
  readout.Pos.YPos = 0;
  EXPECT_EQ(geom->calcPixel(readout), 512); // 0*512 + 511 + 1

  readout.Pos.XPos = 0;
  readout.Pos.YPos = 511;
  EXPECT_EQ(geom->calcPixel(readout), 511 * 512 + 1); // 511*512 + 0 + 1

  readout.Pos.XPos = 511;
  readout.Pos.YPos = 511;
  EXPECT_EQ(geom->calcPixel(readout), 512 * 512); // 511*512 + 511 + 1

  // Test a middle pixel
  readout.Pos.XPos = 256;
  readout.Pos.YPos = 257;
  EXPECT_EQ(geom->calcPixel(readout), 257 * 512 + 256 + 1); // Y*Width + X + 1

  EXPECT_EQ(geom->getBaseCounters().PixelErrors, 0);
}

TEST_F(Geometry2DTest, DifferentDimensions) {
  // Test geometry with smaller dimensions (256x128)
  Config smallConfig;
  smallConfig.CbmParms.MonitorRing = 11;
  smallConfig.CbmParms.MaxRing = 11;
  smallConfig.CbmParms.MaxFENId = 23;
  smallConfig.TopologyMapPtr = std::make_unique<HashMap2D<Topology>>(24);
  std::unique_ptr<Topology> nullTopo = nullptr;
  smallConfig.TopologyMapPtr->add(3, 0, nullTopo);

  auto smallGeom =
      std::make_unique<Geometry2D>(Stats, smallConfig, "small", 256, 128);

  EXPECT_EQ(smallGeom->getWidth(), 256);
  EXPECT_EQ(smallGeom->getHeight(), 128);

  Parser::CbmReadout readout{};
  readout.FiberId = 22; // Valid Ring
  readout.FENId = 3;
  readout.Channel = 0;

  // Valid for 256x128
  readout.Pos.XPos = 255;
  readout.Pos.YPos = 127;
  EXPECT_TRUE(smallGeom->validateReadoutData(readout));
  EXPECT_EQ(smallGeom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(smallGeom->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(smallGeom->getBaseCounters().TopologyError, 0);
  EXPECT_EQ(smallGeom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(smallGeom->getGeometryCounters().XPosErrors, 0);
  EXPECT_EQ(smallGeom->getGeometryCounters().YPosErrors, 0);
  EXPECT_EQ(smallGeom->getBaseCounters().ValidationErrors, 0);
  EXPECT_NE(smallGeom->calcPixel(readout), 0);
  EXPECT_EQ(smallGeom->getBaseCounters().PixelErrors, 0);

  // Invalid for 256x128
  readout.Pos.XPos = 256;
  readout.Pos.YPos = 128;
  EXPECT_FALSE(smallGeom->validateReadoutData(readout));
  EXPECT_EQ(smallGeom->getGeometryCounters().XPosErrors, 1);
  EXPECT_EQ(smallGeom->getGeometryCounters().YPosErrors, 0);
  EXPECT_EQ(smallGeom->getBaseCounters().ValidationErrors, 1);
  EXPECT_EQ(smallGeom->calcPixel(readout), 0);
  EXPECT_EQ(smallGeom->getBaseCounters().PixelErrors, 1);
}

TEST_F(Geometry2DTest, CustomMaxRingAndFENLimits) {
  // Test validation with non-default MaxRing and MaxFEN limits
  Config customConfig;

  // We set a specific ring max ring limit and matching monitor ring
  int MaxRing = 5;

  customConfig.CbmParms.MaxRing = MaxRing;
  customConfig.CbmParms.MonitorRing = MaxRing;
  customConfig.CbmParms.MaxFENId = 10;

  customConfig.TopologyMapPtr = std::make_unique<HashMap2D<Topology>>(24);
  std::unique_ptr<Topology> nullTopo = nullptr;
  customConfig.TopologyMapPtr->add(5, 5, nullTopo);

  auto customGeom =
      std::make_unique<Geometry2D>(Stats, customConfig, "custom", 256, 256);

  ASSERT_EQ(customGeom->getWidth(), 256);
  ASSERT_EQ(customGeom->getHeight(), 256);
  ASSERT_EQ(customGeom->getSourceName(), "custom");

  Parser::CbmReadout readout{};
  readout.FiberId = 10; // Valid Ring
  readout.FENId = 5;
  readout.Channel = 5;
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 100;

  // This readout should be valid for the custom geometry
  EXPECT_TRUE(customGeom->validateReadoutData(readout));
  EXPECT_EQ(customGeom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(customGeom->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(customGeom->getBaseCounters().TopologyError, 0);
  EXPECT_EQ(customGeom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(customGeom->getGeometryCounters().XPosErrors, 0);
  EXPECT_EQ(customGeom->getGeometryCounters().YPosErrors, 0);
  EXPECT_EQ(customGeom->getBaseCounters().ValidationErrors, 0);

  // Test Ring exceeds custom MaxRing limit (FiberId=12 -> Ring=6, MaxRing=5)
  readout.FiberId = 12;
  EXPECT_FALSE(customGeom->validateReadoutData(readout));
  EXPECT_EQ(customGeom->getBaseCounters().RingErrors, 1);
  EXPECT_EQ(customGeom->getBaseCounters().ValidationErrors, 1);

  // Test FEN exceeds custom MaxFEN limit (FENId=11, MaxFENId=10)
  readout.FiberId = 10; // Valid Ring again
  readout.FENId = 11;
  EXPECT_FALSE(customGeom->validateReadoutData(readout));
  EXPECT_EQ(customGeom->getBaseCounters().FENErrors, 1);
  EXPECT_EQ(customGeom->getBaseCounters().ValidationErrors, 2);
}

TEST_F(Geometry2DTest, MetricNamesWithSourcePrefix) {
  // Verify multiple geometry instances have independent, prefixed metrics
  Config config1, config2;
  config1.CbmParms.MonitorRing = 11;
  config1.CbmParms.MaxRing = 11;
  config1.CbmParms.MaxFENId = 23;
  config2.CbmParms.MonitorRing = 11;
  config2.CbmParms.MaxRing = 11;
  config2.CbmParms.MaxFENId = 23;
  config1.TopologyMapPtr = std::make_unique<HashMap2D<Topology>>(24);
  config2.TopologyMapPtr = std::make_unique<HashMap2D<Topology>>(24);
  std::unique_ptr<Topology> nullTopo1 = nullptr;
  config1.TopologyMapPtr->add(5, 5, nullTopo1);
  std::unique_ptr<Topology> nullTopo2 = nullptr;
  config2.TopologyMapPtr->add(5, 5, nullTopo2);

  auto geom1 = std::make_unique<Geometry2D>(Stats, config1, "cbm7", 512, 512);
  auto geom2 = std::make_unique<Geometry2D>(Stats, config2, "cbm8", 512, 512);

  Parser::CbmReadout readout{};
  readout.FiberId = 22; // Valid Ring
  readout.FENId = 5;
  readout.Channel = 5;
  readout.Pos.XPos = 1000; // Invalid
  readout.Pos.YPos = 100;

  geom1->validateReadoutData(readout);
  geom1->validateReadoutData(readout);

  readout.Pos.XPos = 100;
  readout.Pos.YPos = 1000; // Invalid
  geom2->validateReadoutData(readout);

  // Check that each geometry has its own counters
  EXPECT_EQ(geom1->getGeometryCounters().XPosErrors, 2);
  EXPECT_EQ(geom1->getGeometryCounters().YPosErrors, 0);
  EXPECT_EQ(geom1->getBaseCounters().ValidationErrors, 2);
  EXPECT_EQ(geom1->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom1->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(geom1->getBaseCounters().TopologyError, 0);
  EXPECT_EQ(geom1->getGeometryCounters().MonitorRingMismatchErrors, 0);

  EXPECT_EQ(geom2->getGeometryCounters().XPosErrors, 0);
  EXPECT_EQ(geom2->getGeometryCounters().YPosErrors, 1);
  EXPECT_EQ(geom2->getBaseCounters().ValidationErrors, 1);
  EXPECT_EQ(geom2->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom2->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(geom2->getBaseCounters().TopologyError, 0);
  EXPECT_EQ(geom2->getGeometryCounters().MonitorRingMismatchErrors, 0);

  // Verify metrics are accessible by name with proper prefix
  EXPECT_EQ(Stats.getValueByName("cbm7.geometry.xpos_errors"), 2);
  EXPECT_EQ(Stats.getValueByName("cbm7.geometry.ypos_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("cbm8.geometry.xpos_errors"), 0);
  EXPECT_EQ(Stats.getValueByName("cbm8.geometry.ypos_errors"), 1);
}

TEST_F(Geometry2DTest, ValidationOrder) {
  // Test that validation stops at first failure
  Parser::CbmReadout readout{};

  // All invalid: Ring, MonitorRing, FEN, Topology
  // Only Ring error should be counted (first failure)
  readout.FiberId =
      30; // Ring = 15, exceeds MaxRing (also doesn't match MonitorRing)
  readout.FENId = 30;   // Exceeds MaxFEN
  readout.Channel = 99; // Invalid topology

  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 0);     // Not reached
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 0); // Not reached
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors,
            0); // Not reached
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
