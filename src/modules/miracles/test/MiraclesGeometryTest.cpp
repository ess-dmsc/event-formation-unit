

// Copyright (C) 2022 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Miracles position calculations
///
//===----------------------------------------------------------------------===//
#include <common/Statistics.h>
#include <common/testutils/TestBase.h>
#include <memory>
#include <miracles/geometry/MiraclesGeometry.h>

using namespace Caen;

class MiraclesGeometryTest : public TestBase {
protected:
  int TubeA{0};
  int TubeB{1};
  Statistics Stats;
  std::unique_ptr<MiraclesGeometry> geom;
  Config CaenConfiguration;

  void SetUp() override {
    CaenConfiguration.CaenParms.MaxRing = 2; // Miracles has 3 rings, but we use 0-2 for 3 rings
    CaenConfiguration.CaenParms.Resolution = 128;
    // Set MaxAmpl to 40000 - high enough for normal tests but low enough to test the limit
    geom = std::make_unique<MiraclesGeometry>(Stats, CaenConfiguration, 40000);
  }

  void TearDown() override {}
};

TEST_F(MiraclesGeometryTest, Corner) {
  ASSERT_EQ(1, geom->xCoord(0, 0, 0, 5));
  ASSERT_EQ(0, geom->yCoord(0, 0, 5));

  ASSERT_EQ(46, geom->xCoord(1, 11, 5, 0));
  ASSERT_EQ(0, geom->yCoord(1, 5, 0));
  ASSERT_EQ(64, geom->yCoord(2, 5, 0));
}

TEST_F(MiraclesGeometryTest, PosAlongTube) {
  printf("B top\n");
  ASSERT_EQ(geom->tubeAorB(0, 1), TubeB); // 1 is B
  ASSERT_EQ(geom->posAlongUnit(0, 1), 0); // tube B - top 'pixel'
  printf("A bottom\n");
  ASSERT_EQ(geom->tubeAorB(1, 1), TubeA);  // 0 is A
  ASSERT_EQ(geom->posAlongUnit(1, 1), 63); // tube A - bottom 'pixel'
  printf("A bottom\n");
  EXPECT_EQ(geom->tubeAorB(101, 100), TubeA);  // 0 is A
  EXPECT_EQ(geom->posAlongUnit(101, 100), 63); // tube A - bottom 'pixel'
  printf("A top\n");
  EXPECT_EQ(geom->tubeAorB(800, 1), TubeA); // 0 is A
  EXPECT_EQ(geom->posAlongUnit(800, 1), 0); // tube A - top 'pixel'
}

TEST_F(MiraclesGeometryTest, ValidateDataOk) {
  // Test valid data case with non-zero amplitudes
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0};
  ASSERT_TRUE(geom->validateReadoutData(readout));
  
  // Ensure no error counters were incremented
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeHigh, 0);
}

TEST_F(MiraclesGeometryTest, ValidateDataInvalidRing) {
  // FiberId=11 gives Ring=5, which exceeds MaxRing=2
  DataParser::CaenReadout readout{11, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0};
  ASSERT_FALSE(geom->validateReadoutData(readout));
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 1);
  // Ensure no other counters were incremented
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeHigh, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);

  // FiberId=6 gives Ring=3, which exceeds MaxRing=2
  DataParser::CaenReadout readout2{6, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0};
  ASSERT_FALSE(geom->validateReadoutData(readout2));
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 2);
  // Ensure no other counters were incremented
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeHigh, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
}

TEST_F(MiraclesGeometryTest, ValidateDataAmplitudeZero) {
  // Both AmpA and AmpB are zero
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_FALSE(geom->validateReadoutData(readout));
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 1);
  // Ensure no other counters were incremented
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeHigh, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
}

TEST_F(MiraclesGeometryTest, ValidateDataAmplitudeHigh) {
  // Test with amplitudes that exceed MaxAmpl (20001 + 20001 = 40002 > 40000)
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 20001, 20001, 0, 0};
  ASSERT_FALSE(geom->validateReadoutData(readout));
  ASSERT_EQ(geom->getCaenCounters().AmplitudeHigh, 1);
  
  // Test with amplitudes within limit (10000 + 10000 = 20000 < 40000)
  DataParser::CaenReadout readout2{0, 0, 0, 0, 0, 0, 0, 0, 10000, 10000, 0, 0};
  ASSERT_TRUE(geom->validateReadoutData(readout2));
  ASSERT_EQ(geom->getCaenCounters().AmplitudeHigh, 1); // Should not increment
  
  // Ensure no other counters were incremented
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
}

TEST_F(MiraclesGeometryTest, ValidateDataAllCounters) {
  // Verify all counters start at zero
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeZero, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeHigh, 0);
  ASSERT_EQ(geom->getCaenCounters().AmplitudeLow, 0);
  ASSERT_EQ(geom->getCaenCounters().GroupErrors, 0);
}

TEST_F(MiraclesGeometryTest, PosAlongUnitZeroAmplitude) {
  // Test zero amplitude case - defensive check in posAlongUnit should return -1
  ASSERT_EQ(geom->posAlongUnit(0, 0), -1);
  // Verify the ZeroDivError counter was incremented
  ASSERT_EQ(geom->getCaenCounters().ZeroDivError, 1);
}

TEST_F(MiraclesGeometryTest, CalcPixel) {
  // Zero amplitude - should fail validation and return 0
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout), 0);
  // Note: Validation happens before calcPixel, AmplitudeZero counter incremented during validation

  // Valid amplitude - should calculate pixel
  DataParser::CaenReadout readout2{0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0};
  ASSERT_EQ(geom->calcPixel(readout2), 2);
  
  // Ensure no additional error counters were incremented for valid data
  ASSERT_EQ(geom->getBaseCounters().RingErrors, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
