// Copyright (C) 2025 - 2026 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for BeerBase
///
//===----------------------------------------------------------------------===//

#include <beer/BeerBase.h>
#include <cbm/CbmTypes.h>
#include <cinttypes>
#include <common/testutils/TestBase.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using std::filesystem::path;

// clang-format off
// Valid packet with EVENT_2D readout (should be accepted by BEER)
std::vector<uint8_t> ValidBeerPacket {
                0x00, 0x01, // pad, v1
    0x45, 0x53, 0x53, 0x50, // 'E', 'S', 'S', type 0x50 (BEER)
    0x48, 0x00, 0x0B, 0x00, // len(72 0x0048), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x00, 0x00, 0x00, 0x00, // Seq number 0
    0x00, 0x00,             // CMAC Padding

  // Valid EVENT_2D readout
  0x10, 0x00, 0x14, 0x00,  // Data Header: Fiber 16, FEN 0, Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x01, 0x00, 0x00, 0x00,  // Time LO 1 tick
  0x02, 0x00, 0x00, 0x00,  // Type 2 (EVENT_2D), Ch 0, ADC 0
  0x64, 0x00, 0x32, 0x00,  // XPos 100, YPos 50

  // Another valid EVENT_2D readout
  0x10, 0x01, 0x14, 0x00,  // Data Header: Fiber 16, FEN 1, Length 20
  0x00, 0x00, 0x00, 0x00,  // Time HI 0 s
  0x02, 0x00, 0x00, 0x00,  // Time LO 2 ticks
  0x02, 0x01, 0x00, 0x00,  // Type 2 (EVENT_2D), Ch 1, ADC 0
  0xC8, 0x00, 0x96, 0x00   // XPos 200, YPos 150
};

// Create packet with CBM detector type (0x10) instead of BEER (0x50)
std::vector<uint8_t> CBMTypePacket {
              0x00, 0x01, // pad, v1
  0x45, 0x53, 0x53, 0x10, // 'E', 'S', 'S', type 0x10 (CBM - wrong!)
  0x20, 0x00, 0x0B, 0x00, // len(32 0x0020), OQ11, TSrc0
  0x00, 0x00, 0x00, 0x00, // PT HI
  0x00, 0x00, 0x00, 0x00, // PT LO
  0x00, 0x00, 0x00, 0x00, // PPT HI
  0x00, 0x00, 0x00, 0x00, // PPT Lo
  0x00, 0x00, 0x00, 0x00, // Seq number 0
  0x00, 0x00,             // CMAC Padding
};
// clang-format on

using namespace beer;

class BeerBaseTest : public ::testing::Test {
public:
  BaseSettings Settings;

  void SetUp() override {
    // Get base test dir
    path TestDir = path(__FILE__).parent_path();

    Settings.DetectorName = "beer";
    Settings.GraphitePrefix = "beer";
    Settings.GraphiteRegion = "test";
    Settings.ConfigFile = TestDir / path("beer_base_test.json");
    Settings.KafkaTopic = "beer_detector";
    Settings.NoHwCheck = true;
  }

  void TearDown() override {}
};

TEST_F(BeerBaseTest, Constructor) {
  BeerBase DetectorBase(Settings);
  ASSERT_EQ(DetectorBase.getStatPrefix(1), "beer.test.");
  EXPECT_EQ(DetectorBase.getInputCounters().RxPackets, 0);
  EXPECT_EQ(DetectorBase.Counters.CbmStats.Readouts, 0);
}

TEST_F(BeerBaseTest, AcceptsValidEvent2D) {
  BeerBase DetectorBase(Settings);

  writePacketToRxFIFO(DetectorBase, ValidBeerPacket);

  // ESS header should have correct detector type (no type errors)
  EXPECT_EQ(DetectorBase.getStatValueByName("parser.essheader.errors.type"), 0);

  // Both EVENT_2D readouts should be accepted
  EXPECT_EQ(DetectorBase.Counters.CbmStats.Readouts, 2);
  EXPECT_EQ(DetectorBase.Counters.CbmStats.ErrorType, 0);

  DetectorBase.stopThreads();
}

TEST_F(BeerBaseTest, RejectsCBMDetectorType) {

  BeerBase DetectorBase(Settings);

  writePacketToRxFIFO(DetectorBase, CBMTypePacket);

  // Packet should be rejected due to wrong detector type in ESS header
  EXPECT_EQ(DetectorBase.getStatValueByName("parser.essheader.errors.type"), 1);

  // Readout is not  processed at all due to wrong detector type, so no readouts
  // or type errors should be counted
  EXPECT_EQ(DetectorBase.Counters.CbmStats.Readouts, 0);
  EXPECT_EQ(DetectorBase.Counters.CbmStats.ErrorType, 0);

  DetectorBase.stopThreads();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
