// Copyright (C) 2019 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for LokiGeometry class
///
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <algorithm>
#include <common/testutils/TestBase.h>
#include <loki/geometry/LokiConfig.h>
#include <loki/geometry/LokiGeometry.h>
#include <memory>

using namespace Caen;

class LokiGeometryTest : public TestBase {
protected:
  Statistics Stats;
  LokiGeometry *geom;
  Config config{LOKI_CONFIG};

  void SetUp() override {
    config.parseConfig();
    geom = new LokiGeometry(Stats, config);
    // geom->setResolution(512);
    geom->CaenCDCalibration = CDCalibration("loki", LOKI_CALIB);
    geom->CaenCDCalibration.parseCalibration();
  }

  void TearDown() override {}
};

TEST_F(LokiGeometryTest, Default) {
  ASSERT_EQ(config.LokiConf.Parms.ConfiguredBanks, 9);
  ASSERT_EQ(config.LokiConf.Parms.ConfiguredRings, 10);
}

TEST_F(LokiGeometryTest, getY) {
  //                          Ring, FEN, Group, Unit
  ASSERT_EQ(config.LokiConf.getY(2, 0, 0, 0), 1568);
  ASSERT_EQ(config.LokiConf.getY(3, 0, 0, 0), 2016);
}

TEST_F(LokiGeometryTest, Bank0FirstPixelFirstStraw) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  readout.FiberId = 0; // Ring 0
  readout.FENId = 0;   // First Module of 8 Tubes
  readout.Group = 0;   // First Tube

  // Ensure Unit == 0 and Group == 0
  readout.AmpA = 0;
  readout.AmpB = 0;
  readout.AmpC = 100;
  readout.AmpD = 0;
  ASSERT_EQ(geom->calcPixel(readout), 1);
}

TEST_F(LokiGeometryTest, Bank1FirstPixelFirstStraw) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  readout.FiberId = 4; // Ring 1
  readout.FENId = 0;   // First Module of 8 Tubes
  readout.Group = 0;   // First Tube in Module

  // Ensure Unit == 0 and Group == 0
  readout.AmpA = 0;
  readout.AmpB = 0;
  readout.AmpC = 100;
  readout.AmpD = 0;
  ASSERT_EQ(geom->calcPixel(readout), 802817);
}

TEST_F(LokiGeometryTest, Bank2FirstPixelFirstStraw) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  readout.FiberId = 6; // Ring 3 - Bank 2
  readout.FENId = 0;   // First Module of 8 Tubes
  readout.Group = 0;   // First Tube in Module

  // Ensure Unit == 0 and Group == 0
  readout.AmpA = 0;
  readout.AmpB = 0;
  readout.AmpC = 100;
  readout.AmpD = 0;
  ASSERT_EQ(geom->calcPixel(readout), 1032193);
}

TEST_F(LokiGeometryTest, Bank0LastPixelFirstStraw) {
  DataParser::CaenReadout readout{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  readout.FiberId = 0; // Ring 0
  readout.FENId = 0;   // First Module of 8 Tubes
  readout.Group = 0;   // First Tube

  // Ensure Unit == 1.0 and Group == 0
  readout.AmpA = 100;
  readout.AmpB = 0;
  readout.AmpC = 0;
  readout.AmpD = 0;
  ASSERT_EQ(geom->calcPixel(readout), 512);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
