// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for CBM unified geometry with topology cache
///
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <memory>
#include <modules/cbm/CbmTypes.h>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/geometry/Geometry.h>
#include <modules/cbm/readout/Parser.h>

using namespace cbm;

class GeometryTest : public TestBase {
protected:
  Statistics Stats;
  Config CbmConfig;
  std::unique_ptr<Geometry> geom;

  void SetUp() override {
    CbmConfig.CbmParms.MonitorRing = 11;
    CbmConfig.CbmParms.MaxRing = 11;
    CbmConfig.CbmParms.MaxFENId = 23;

    // Create topology configuration with multiple monitors of same type
    CbmConfig.TopologyMapPtr = std::make_unique<HashMap2D<Topology>>(24);

    // Add two EVENT_2D monitors at different FEN/Channel positions
    auto topo2D_1 = std::make_unique<Topology>(
        3, 0, "cbm_2d_1", CbmType::EVENT_2D, SchemaType::EV44, 512, 512, 0, 0);
    CbmConfig.TopologyMapPtr->add(3, 0, topo2D_1);

    auto topo2D_2 = std::make_unique<Topology>(
        3, 1, "cbm_2d_2", CbmType::EVENT_2D, SchemaType::EV44, 256, 256, 0, 0);
    CbmConfig.TopologyMapPtr->add(3, 1, topo2D_2);

    // Add two EVENT_0D monitors at different FEN/Channel positions
    auto topo0D_1 = std::make_unique<Topology>(
        4, 0, "cbm_0d_1", CbmType::EVENT_0D, SchemaType::EV44, 100);
    CbmConfig.TopologyMapPtr->add(4, 0, topo0D_1);

    auto topo0D_2 = std::make_unique<Topology>(
        4, 1, "cbm_0d_2", CbmType::EVENT_0D, SchemaType::DA00, 200);
    CbmConfig.TopologyMapPtr->add(4, 1, topo0D_2);

    // Add IBM monitor
    auto topoIBM = std::make_unique<Topology>(5, 0, "cbm_ibm", CbmType::IBM,
                                              SchemaType::DA00, 0);
    CbmConfig.TopologyMapPtr->add(5, 0, topoIBM);

    // Create unified geometry
    geom = std::make_unique<Geometry>(Stats, CbmConfig);
  }
};

//=============================================================================
// Constructor and Cached Topology Tests
//=============================================================================

TEST_F(GeometryTest, Constructor) { ASSERT_NE(geom, nullptr); }

TEST_F(GeometryTest, CachedTopologyCreated) {
  // Check 2D cached topology #1 exists
  const auto *cached2D_1 = geom->getCachedTopology(3, 0);
  ASSERT_NE(cached2D_1, nullptr);
  EXPECT_EQ(cached2D_1->Type, CbmType::EVENT_2D);
  EXPECT_EQ(cached2D_1->ESSGeom.nx(), 512);
  EXPECT_EQ(cached2D_1->ESSGeom.ny(), 512);

  // Check 2D cached topology #2 exists
  const auto *cached2D_2 = geom->getCachedTopology(3, 1);
  ASSERT_NE(cached2D_2, nullptr);
  EXPECT_EQ(cached2D_2->Type, CbmType::EVENT_2D);
  EXPECT_EQ(cached2D_2->ESSGeom.nx(), 256);
  EXPECT_EQ(cached2D_2->ESSGeom.ny(), 256);

  // Check 0D cached topology #1 exists
  const auto *cached0D_1 = geom->getCachedTopology(4, 0);
  ASSERT_NE(cached0D_1, nullptr);
  EXPECT_EQ(cached0D_1->Type, CbmType::EVENT_0D);
  EXPECT_EQ(cached0D_1->PixelOffset, 100);

  // Check 0D cached topology #2 exists
  const auto *cached0D_2 = geom->getCachedTopology(4, 1);
  ASSERT_NE(cached0D_2, nullptr);
  EXPECT_EQ(cached0D_2->Type, CbmType::EVENT_0D);
  EXPECT_EQ(cached0D_2->PixelOffset, 200);

  // Check IBM cached topology exists
  const auto *cachedIBM = geom->getCachedTopology(5, 0);
  ASSERT_NE(cachedIBM, nullptr);
  EXPECT_EQ(cachedIBM->Type, CbmType::IBM);

  // Non-existent topology
  EXPECT_EQ(geom->getCachedTopology(99, 99), nullptr);
}

//=============================================================================
// 2D Validation Tests
//=============================================================================

TEST_F(GeometryTest, Validate2DValidReadouts) {
  Parser::CbmReadout readout{};
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D);
  readout.FiberId = 22;
  readout.FENId = 3;
  readout.Channel = 0;
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 200;

  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);
}

TEST_F(GeometryTest, Validate2DBoundaryCoordinates) {
  Parser::CbmReadout readout{};
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D);
  readout.FiberId = 22;
  readout.FENId = 3;
  readout.Channel = 0;

  // Test corners for 512x512
  readout.Pos.XPos = 0;
  readout.Pos.YPos = 0;
  EXPECT_TRUE(geom->validateReadoutData(readout));

  readout.Pos.XPos = 511;
  readout.Pos.YPos = 511;
  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);

  // Test corners for 256x256
  readout.Channel = 1;
  readout.Pos.XPos = 0;
  readout.Pos.YPos = 0;
  EXPECT_TRUE(geom->validateReadoutData(readout));

  readout.Pos.XPos = 255;
  readout.Pos.YPos = 255;
  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);
}

TEST_F(GeometryTest, Validate2DMultipleMonitors) {
  Parser::CbmReadout readout{};
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D);
  readout.FiberId = 22;

  // Error on first 2D monitor (FEN 3, Channel 0, 512x512)
  readout.FENId = 3;
  readout.Channel = 0;
  readout.Pos.XPos = 512; // Out of bounds for 512x512
  readout.Pos.YPos = 100;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().XPosErrors, 1);

  // Error on second 2D monitor (FEN 3, Channel 1, 256x256)
  readout.FENId = 3;
  readout.Channel = 1;
  readout.Pos.XPos = 256; // Out of bounds for 256x256
  readout.Pos.YPos = 100;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().XPosErrors, 2); // Accumulated

  // Y error on first monitor
  readout.FENId = 3;
  readout.Channel = 0;
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 513; // Out of bounds for 512x512
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().YPosErrors, 1);

  // Y error on second monitor
  readout.FENId = 3;
  readout.Channel = 1;
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 256; // Out of bounds for 256x256
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().YPosErrors, 2); // Accumulated

  // Total validation errors
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 4);
}

//=============================================================================
// 0D Validation Tests
//=============================================================================

TEST_F(GeometryTest, Validate0DMultipleMonitors) {
  Parser::CbmReadout readout{};
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_0D);
  readout.FiberId = 22;

  // Validate first 0D monitor
  readout.FENId = 4;
  readout.Channel = 0;
  EXPECT_TRUE(geom->validateReadoutData(readout));

  // Validate second 0D monitor
  readout.FENId = 4;
  readout.Channel = 1;
  EXPECT_TRUE(geom->validateReadoutData(readout));

  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);
}

//=============================================================================
// IBM Validation Tests
//=============================================================================

TEST_F(GeometryTest, ValidateIBMValidReadout) {
  Parser::CbmReadout readout{};
  readout.Type = static_cast<uint8_t>(CbmType::IBM);
  readout.FiberId = 22;
  readout.FENId = 5;
  readout.Channel = 0;

  EXPECT_TRUE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 0);
}

//=============================================================================
// Common Validation Tests (Ring, FEN, Topology, Type)
// These tests verify that common validations work uniformly across all types
//=============================================================================

TEST_F(GeometryTest, ValidateInvalidRing) {
  Parser::CbmReadout readout{};
  readout.FiberId = 30; // Ring = 15, exceeds MaxRing of 11
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 100;

  // Test with EVENT_2D
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D);
  readout.FENId = 3;
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);

  // Test with EVENT_0D
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_0D);
  readout.FENId = 4;
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 2); // Accumulated
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 2);

  // Test with IBM
  readout.Type = static_cast<uint8_t>(CbmType::IBM);
  readout.FENId = 5;
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().RingErrors, 3); // Accumulated
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 3);
}

TEST_F(GeometryTest, ValidateMonitorRingMismatch) {
  Parser::CbmReadout readout{};
  readout.FiberId = 20; // Ring = 10, doesn't match MonitorRing of 11
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 100;

  // Test with EVENT_2D
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D);
  readout.FENId = 3;
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().MonitorRingMismatchErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);

  // Test with EVENT_0D
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_0D);
  readout.FENId = 4;
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().MonitorRingMismatchErrors, 2); // Accumulated
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 2);

  // Test with IBM
  readout.Type = static_cast<uint8_t>(CbmType::IBM);
  readout.FENId = 5;
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().MonitorRingMismatchErrors, 3); // Accumulated
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 3);
}

TEST_F(GeometryTest, ValidateInvalidFEN) {
  Parser::CbmReadout readout{};
  readout.FiberId = 22; // Valid ring
  readout.FENId = 30;   // Exceeds MaxFENId of 23
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 100;

  // Test with EVENT_2D
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D);
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);

  // Test with EVENT_0D
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_0D);
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 2); // Accumulated
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 2);

  // Test with IBM
  readout.Type = static_cast<uint8_t>(CbmType::IBM);
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().FENErrors, 3); // Accumulated
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 3);
}

TEST_F(GeometryTest, ValidateInvalidTopology) {
  Parser::CbmReadout readout{};
  readout.FiberId = 22;
  readout.FENId = 10; // FEN/Channel not in topology
  readout.Channel = 5;
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 100;

  // Test with EVENT_2D
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D);
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 1);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);

  // Test with EVENT_0D
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_0D);
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 2);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 2);

  // Test with IBM
  readout.Type = static_cast<uint8_t>(CbmType::IBM);
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getBaseCounters().TopologyError, 3);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 3);
}

TEST_F(GeometryTest, ValidateTypeMismatch) {
  Parser::CbmReadout readout{};
  readout.FiberId = 22;
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 100;

  // Wrong type for first 2D monitor (FEN 3, Channel 0)
  readout.Type = static_cast<uint8_t>(CbmType::IBM); // Should be EVENT_2D
  readout.FENId = 3;
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().TypeMismatchErrors, 1);
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 1);

  // Wrong type for second 2D monitor (FEN 3, Channel 1)
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_0D); // Should be EVENT_2D
  readout.FENId = 3;
  readout.Channel = 1;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().TypeMismatchErrors, 2); // Accumulated
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 2);

  // Wrong type for first 0D monitor (FEN 4, Channel 0)
  readout.Type = static_cast<uint8_t>(CbmType::IBM); // Should be EVENT_0D
  readout.FENId = 4;
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().TypeMismatchErrors, 3); // Accumulated
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 3);

  // Wrong type for second 0D monitor (FEN 4, Channel 1)
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D); // Should be EVENT_0D
  readout.FENId = 4;
  readout.Channel = 1;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().TypeMismatchErrors, 4); // Accumulated
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 4);

  // Wrong type for IBM monitor (FEN 5, Channel 0)
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D); // Should be IBM
  readout.FENId = 5;
  readout.Channel = 0;
  EXPECT_FALSE(geom->validateReadoutData(readout));
  EXPECT_EQ(geom->getCbmCounters().TypeMismatchErrors, 5); // Accumulated
  EXPECT_EQ(geom->getBaseCounters().ValidationErrors, 5);
}

//=============================================================================
// Pixel Calculation Tests
//=============================================================================

TEST_F(GeometryTest, CalcPixel2D) {
  Parser::CbmReadout readout{};
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D);
  readout.FiberId = 22;
  readout.FENId = 3;
  readout.Channel = 0; // 512x512 monitor

  // Create reference ESSGeometry for comparison
  ESSGeometry refGeom(512, 512, 1, 1);

  readout.Pos.XPos = 0;
  readout.Pos.YPos = 0;
  uint32_t pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, refGeom.pixel2D(0, 0));

  readout.Pos.XPos = 1;
  readout.Pos.YPos = 0;
  pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, refGeom.pixel2D(1, 0));

  readout.Pos.XPos = 0;
  readout.Pos.YPos = 1;
  pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, refGeom.pixel2D(0, 1));

  readout.Pos.XPos = 100;
  readout.Pos.YPos = 200;
  pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, refGeom.pixel2D(100, 200));
}

TEST_F(GeometryTest, CalcPixel0D) {
  Parser::CbmReadout readout{};
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_0D);
  readout.FiberId = 22;
  readout.FENId = 4;
  readout.Channel = 0; // offset 100

  uint32_t pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, 100);
}

TEST_F(GeometryTest, CalcPixelIBM) {
  Parser::CbmReadout readout{};
  readout.Type = static_cast<uint8_t>(CbmType::IBM);
  readout.FiberId = 22;
  readout.FENId = 5;
  readout.Channel = 0;

  uint32_t pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, 0); // IBM returns 0 (pixel not used)
}

TEST_F(GeometryTest, CalcPixelNoTopology) {
  Parser::CbmReadout readout{};
  readout.FENId = 99;
  readout.Channel = 99;

  uint32_t pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, 0);
  EXPECT_EQ(geom->getBaseCounters().PixelErrors, 1);
}

TEST_F(GeometryTest, CalcPixel2DInvalidCoordinates) {
  Parser::CbmReadout readout{};
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_2D);
  readout.FiberId = 22;
  readout.FENId = 3;
  readout.Channel = 0; // 512x512 monitor

  // Invalid X coordinate (out of bounds)
  readout.Pos.XPos = 512;
  readout.Pos.YPos = 100;
  uint32_t pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, 0);
  EXPECT_EQ(geom->getBaseCounters().PixelErrors, 1);

  // Invalid Y coordinate (out of bounds)
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 512;
  pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, 0);
  EXPECT_EQ(geom->getBaseCounters().PixelErrors, 2); // Accumulated

  readout.FENId = 3;
  readout.Channel = 99; // No topology at this channel
  readout.Pos.XPos = 100;
  readout.Pos.YPos = 100;
  pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, 0);
  EXPECT_EQ(geom->getBaseCounters().PixelErrors, 3); // Accumulated
}

TEST_F(GeometryTest, CalcPixel0DInvalidTopology) {
  Parser::CbmReadout readout{};
  readout.Type = static_cast<uint8_t>(CbmType::EVENT_0D);
  readout.FiberId = 22;
  readout.FENId = 99; // No topology at this FEN/Channel
  readout.Channel = 99;

  uint32_t pixel = geom->calcPixel(readout);
  EXPECT_EQ(pixel, 0);
  EXPECT_EQ(geom->getBaseCounters().PixelErrors, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
