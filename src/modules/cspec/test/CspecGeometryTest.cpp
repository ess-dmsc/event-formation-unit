// Copyright (C) 2023 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for CSPEC position calculations
///
//===----------------------------------------------------------------------===//
#include <common/Statistics.h>
#include <caen/readout/DataParser.h>
#include <common/testutils/TestBase.h>
#include <cspec/geometry/CspecGeometry.h>
#include <gtest/gtest.h>
#include <memory>

using namespace Caen;

class CspecGeometryTest : public TestBase {
protected:
  Statistics Stats;
  Config CaenConfiguration;
  std::unique_ptr<CspecGeometry> geom;

  void SetUp() override {
    CaenConfiguration.CaenParms.MaxRing = 4;
    CaenConfiguration.CaenParms.Resolution = 300;
    geom = std::make_unique<CspecGeometry>(Stats, CaenConfiguration);
  }

  void TearDown() override {}
};

///\todo Replace CSPEC geometry YOffset tests once a new ICD is decided for 3He
/// CSPEC
// TEST_F(CspecGeometryTest, YOffset) {
//   ASSERT_EQ(geom->yOffset(0), 0);
//   ASSERT_EQ(geom->yOffset(1), 0);
//   ASSERT_EQ(geom->yOffset(2), 0);
//   ASSERT_EQ(geom->yOffset(3), 3);
//   ASSERT_EQ(geom->yOffset(4), 3);
//   ASSERT_EQ(geom->yOffset(5), 3);
//   ASSERT_EQ(geom->yOffset(6), 6);
//   ASSERT_EQ(geom->yOffset(7), 6);
//   ASSERT_EQ(geom->yOffset(8), 6);
//   ASSERT_EQ(geom->yOffset(9), 9);
//   ASSERT_EQ(geom->yOffset(10), 9);
//   ASSERT_EQ(geom->yOffset(11), 9);
//   ASSERT_EQ(geom->yOffset(12), 12);
//   ASSERT_EQ(geom->yOffset(13), 12);
//   ASSERT_EQ(geom->yOffset(14), 12);
// }

TEST_F(CspecGeometryTest, YCoord) {
  ASSERT_EQ(geom->yCoord(0, 0), -1);
  ASSERT_EQ(geom->yCoord(1, 0), geom->Resolution - 1);
  ASSERT_EQ(geom->yCoord(0, 1), 0);
}

///\todo Replace CSPEC geometry XOffset tests once a new ICD is decided for 3He
/// CSPEC
// TEST_F(CspecGeometryTest, XOffset) {
//   ASSERT_EQ(geom->xOffset(0, 0), 0);
//   ASSERT_EQ(geom->xOffset(0, 1), 100);
//   ASSERT_EQ(geom->xOffset(0, 2), 200);
//   ASSERT_EQ(geom->xOffset(1, 0), 300);
//   ASSERT_EQ(geom->xOffset(1, 1), 400);
//   ASSERT_EQ(geom->xOffset(1, 2), 500);
//   ASSERT_EQ(geom->xOffset(2, 0), 600);
//   ASSERT_EQ(geom->xOffset(2, 1), 700);
//   ASSERT_EQ(geom->xOffset(2, 2), 800);
// }

TEST_F(CspecGeometryTest, Position) {
  ASSERT_EQ(geom->posAlongUnit(0, 0), -1);
  ASSERT_EQ(geom->posAlongUnit(0, 1), 0);
  ASSERT_EQ(geom->posAlongUnit(1, 0), geom->Resolution - 1);
}

TEST_F(CspecGeometryTest, CalcPixel) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);

  DataParser::CaenReadout readout2{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout2), 1);
}

///\todo Fix CSPEC geometry tests once a new ICD is decided for 3He CSPEC
TEST_F(CspecGeometryTest, Validate) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_TRUE(geom->validateReadoutData(readout));

  readout.FiberId = 10;
  ASSERT_FALSE(geom->validateReadoutData(readout));
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 1);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);

  readout.FiberId = 0;
  readout.FENId = 20;
  ASSERT_FALSE(geom->validateReadoutData(readout));
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 1);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 1);
  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);

  readout.FiberId = 0;
  readout.FENId = 0;
  readout.Group = 20;
  ASSERT_FALSE(geom->validateReadoutData(readout));
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 1);
  ASSERT_EQ(geom->getBaseCounters().FENErrors, 1);
  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 1);
}

TEST_F(CspecGeometryTest, SerializerConfiguration) {
  // Test that CSPEC geometry uses a single serializer
  ASSERT_EQ(geom->numSerializers(), 1);
  
  // Test serializer name at index 0
  ASSERT_EQ(geom->serializerName(0), "caen");

  // Test serializer name at out of range index
  ASSERT_EQ(geom->serializerName(2), "caen"); // out of range index still returns "caen"
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
