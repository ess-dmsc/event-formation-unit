// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for CBM 0D geometry validation
///
//===----------------------------------------------------------------------===//

#include "CbmTypes.h"
#include <common/testutils/TestBase.h>
#include <gtest/gtest.h>
#include <memory>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/geometry/Geometry0D.h>
#include <modules/cbm/readout/Parser.h>

using namespace cbm;

class Geometry0DTest : public TestBase {
protected:
  Statistics Stats;
  Config CbmConfig;
  std::unique_ptr<Geometry0D> geom;

  void SetUp() override {
    CbmConfig.CbmParms.MonitorRing = 11; // Default monitor ring
    CbmConfig.CbmParms.MaxRing = 11;     // Default max ring
    CbmConfig.CbmParms.MaxFENId = 23;    // Default max FEN

    // Create topology configuration
    CbmConfig.TopologyMapPtr = std::make_unique<HashMap2D<Topology>>(24);
    std::unique_ptr<Topology> nullTopo = nullptr;
    CbmConfig.TopologyMapPtr->add(3, 0, nullTopo);

    // Create 0D geometry with pixel offset 100 (EVENT_0D style)
    geom = std::make_unique<Geometry0D>(Stats, CbmConfig, "test_source", 100);
  }

  void TearDown() override {}
};

TEST_F(Geometry0DTest, Constructor) {
  EXPECT_NE(geom, nullptr);
  EXPECT_EQ(geom->getSourceName(), "test_source");
  EXPECT_EQ(geom->getPixelOffset(), 100);
}

TEST_F(Geometry0DTest, ConstructorIBMStyle) {
  // IBM monitors use pixel offset 0
  auto ibmGeom = std::make_unique<Geometry0D>(Stats, CbmConfig, "ibm1");
  EXPECT_EQ(ibmGeom->getPixelOffset(), 0);
  EXPECT_EQ(ibmGeom->getSourceName(), "ibm1");
}

TEST_F(Geometry0DTest, ValidateValidReadoutData) {
  Parser::CbmReadout readout{};

  // Set valid Ring and FEN
  readout.FiberId = 22; // Ring = 22/2 = 11, matches MonitorRing
  readout.FENId = 3;    // Valid topology
  readout.Channel = 0;  // Valid topology

  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 0);
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);
}

TEST_F(Geometry0DTest, ValidateTopology) {
  Parser::CbmReadout readout{};
  readout.FiberId = 22; // Valid Ring

  // Valid topology
  readout.FENId = 3;
  readout.Channel = 0;
  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);

  // Invalid topology - FEN/Channel not in configuration
  readout.FENId = 5;
  readout.Channel = 5;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 1);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 0);

  // Another invalid topology
  readout.FENId = 10;
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 2);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 2);
}

TEST_F(Geometry0DTest, ValidateRingAndFEN) {
  Parser::CbmReadout readout{};
  readout.FENId = 3;   // Valid topology
  readout.Channel = 0; // Valid topology

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
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 1); // FEN validator not reached
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 3);
}

TEST_F(Geometry0DTest, ValidateMonitorRingMismatch) {
  Parser::CbmReadout readout{};
  readout.FENId = 3;   // Valid topology
  readout.Channel = 0; // Valid topology

  // Ring matches MonitorRing (11)
  readout.FiberId = 22; // Ring = 11
  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 0);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);

  // Ring doesn't match MonitorRing (Ring = 10, MonitorRing = 11)
  readout.FiberId = 20; // Ring = 10, valid but doesn't match MonitorRing
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().RingErrors,
            0); // Ring is valid, just mismatched

  // Another mismatch (Ring = 5, MonitorRing = 11)
  readout.FiberId = 10; // Ring = 5
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getGeometryCounters().MonitorRingMismatchErrors, 2);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 2);
}

TEST_F(Geometry0DTest, CalcPixelReturnsFixedOffset) {
  Parser::CbmReadout readout{};

  // For EVENT_0D geometry, calcPixel always returns the fixed pixel offset
  readout.FiberId = 22;
  readout.FENId = 3;
  readout.Channel = 0;

  EXPECT_EQ(geom->calcPixel(readout), 100);

  // Different readout data, same result
  readout.FiberId = 18;
  readout.FENId = 5;
  readout.Channel = 2;
  EXPECT_EQ(geom->calcPixel(readout), 100);

  // Pixel offset is fixed regardless of input
  EXPECT_EQ(geom->getBaseCounters().PixelErrors, 0);
}

TEST_F(Geometry0DTest, CalcPixelIBMStyle) {
  // IBM monitors return 0 (pixel not used for histograms)
  auto ibmGeom = std::make_unique<Geometry0D>(Stats, CbmConfig, "ibm1", 0);

  Parser::CbmReadout readout{};
  readout.FiberId = 22;
  readout.FENId = 3;
  readout.Channel = 0;

  // IBM returns 0 - note that DetectorGeometry counts this as PixelError
  // but IBM doesn't use pixel IDs (uses ADC histograms instead)
  EXPECT_EQ(ibmGeom->calcPixel(readout), 0);
  EXPECT_EQ(ibmGeom->getBaseCounters().PixelErrors, 1);
}

TEST_F(Geometry0DTest, CustomMaxRingAndFENLimits) {
  // Test validation with non-default MaxRing and MaxFEN limits
  Config customConfig;

  int MaxRing = 5;
  customConfig.CbmParms.MaxRing = MaxRing;
  customConfig.CbmParms.MonitorRing = MaxRing;
  customConfig.CbmParms.MaxFENId = 10;

  customConfig.TopologyMapPtr = std::make_unique<HashMap2D<Topology>>(24);
  std::unique_ptr<Topology> nullTopo = nullptr;
  customConfig.TopologyMapPtr->add(5, 5, nullTopo);

  auto customGeom =
      std::make_unique<Geometry0D>(Stats, customConfig, "custom", 50);

  ASSERT_EQ(customGeom->getSourceName(), "custom");
  ASSERT_EQ(customGeom->getPixelOffset(), 50);

  Parser::CbmReadout readout{};
  readout.FiberId = 10; // Ring = 5, matches custom MaxRing and MonitorRing
  readout.FENId = 5;
  readout.Channel = 5;

  // Valid for custom geometry
  EXPECT_TRUE(customGeom->validateReadoutData(readout));
  EXPECT_EQ(customGeom->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(customGeom->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(customGeom->getBaseCounters().TopologyError, 0);
  EXPECT_EQ(customGeom->getGeometryCounters().MonitorRingMismatchErrors, 0);
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

  // Test MonitorRing mismatch with custom limits (Ring=4, MonitorRing=5)
  readout.FiberId = 8; // Ring = 4, valid but doesn't match MonitorRing
  readout.FENId = 5;
  EXPECT_FALSE(customGeom->validateReadoutData(readout));
  EXPECT_EQ(customGeom->getGeometryCounters().MonitorRingMismatchErrors, 1);
  EXPECT_EQ(customGeom->getBaseCounters().ValidationErrors, 3);
}

TEST_F(Geometry0DTest, MetricNamesWithSourcePrefix) {
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

  auto geom1 = std::make_unique<Geometry0D>(Stats, config1, "cbm1", 100);
  auto geom2 = std::make_unique<Geometry0D>(Stats, config2, "cbm2", 200);

  Parser::CbmReadout readout{};
  readout.FiberId = 20; // Ring = 10, doesn't match MonitorRing = 11
  readout.FENId = 5;
  readout.Channel = 5;

  geom1->validateReadoutData(readout);
  geom1->validateReadoutData(readout);

  readout.FiberId = 18; // Ring = 9, doesn't match MonitorRing = 11
  geom2->validateReadoutData(readout);

  // Check that each geometry has its own counters
  EXPECT_EQ(geom1->getGeometryCounters().MonitorRingMismatchErrors, 2);
  EXPECT_EQ(geom1->getBaseCounters().ValidationErrors, 2);
  EXPECT_EQ(geom1->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom1->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(geom1->getBaseCounters().TopologyError, 0);

  EXPECT_EQ(geom2->getGeometryCounters().MonitorRingMismatchErrors, 1);
  EXPECT_EQ(geom2->getBaseCounters().ValidationErrors, 1);
  EXPECT_EQ(geom2->getBaseCounters().RingErrors, 0);
  EXPECT_EQ(geom2->getBaseCounters().FENErrors, 0);
  EXPECT_EQ(geom2->getBaseCounters().TopologyError, 0);

  // Verify metrics are accessible by name with proper prefix
  EXPECT_EQ(Stats.getValueByName("cbm1.geometry.monitor_ring_mismatch_errors"),
            2);
  EXPECT_EQ(Stats.getValueByName("cbm2.geometry.monitor_ring_mismatch_errors"),
            1);
}

TEST_F(Geometry0DTest, ValidationOrder) {
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
