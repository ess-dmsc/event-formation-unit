// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <common/Statistics.h>
#include <dream/geometry/DreamGeometry.h>
#include <dream/geometry/Config.h>
#include <dream/readout/DataParser.h>
#include <memory>

using namespace Dream;

class DreamGeometryTest : public TestBase {
protected:
  DataParser::CDTReadout Readout{0, 0, 0, 0, 0, 0, 0, 0, 0};
  std::unique_ptr<DreamGeometry> geometry;
  Statistics Stats;
  Config DreamConfig;

  void SetUp() override {
    // Initialize config before creating geometry so modules get created
    DreamConfig.RMConfig[0][0].Initialised = true;
    DreamConfig.RMConfig[0][0].Type = Config::ModuleType::BwEndCap;
    
    geometry = std::make_unique<DreamGeometry>(Stats, DreamConfig);
  }
  void TearDown() override {}
};

TEST_F(DreamGeometryTest, PixelOffsets) {
  ASSERT_EQ(geometry->getPixelOffset(Config::FwEndCap), 0);
  ASSERT_EQ(geometry->getPixelOffset(Config::BwEndCap), 71680);
  ASSERT_EQ(geometry->getPixelOffset(Config::DreamMantle), 229376);
  ASSERT_EQ(geometry->getPixelOffset(Config::SANS), 720896);
  ASSERT_EQ(geometry->getPixelOffset(Config::HR), 1122304);
}

TEST_F(DreamGeometryTest, PixelOffsetsError) {
  ASSERT_EQ(geometry->getPixelOffset(Config::PA), -1);
}

TEST_F(DreamGeometryTest, GetPixel) {
  // Reset and configure for BwEndCap test
  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::BwEndCap;
  DreamConfig.RMConfig[0][0].Initialised = true;
  geometry = std::make_unique<DreamGeometry>(Stats, DreamConfig);
  Readout.FiberId = 0;  // Ring = FiberId / 2 = 0
  Readout.FENId = 0;
  Readout.UnitId = 6;
  Readout.Anode = 32;   // Valid anode value
  Readout.Cathode = 32; // Valid cathode value
  auto pixel = geometry->calcPixel(Readout);
  ASSERT_TRUE(pixel >= 71681) << "BwEndCap: Got pixel: " << pixel;

  // Reset and configure for DreamMantle test
  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::DreamMantle;
  DreamConfig.RMConfig[0][0].P2.Cassette = 0;
  DreamConfig.RMConfig[0][0].Initialised = true;
  geometry = std::make_unique<DreamGeometry>(Stats, DreamConfig);
  Readout.Anode = 16;   // Valid anode for mantle
  Readout.Cathode = 16; // Valid cathode for mantle
  pixel = geometry->calcPixel(Readout);
  ASSERT_TRUE(pixel >= 229377) << "DreamMantle: Got pixel: " << pixel;

  // Reset and configure for HR test
  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::HR;
  DreamConfig.RMConfig[0][0].P2.Rotate = 0;
  DreamConfig.RMConfig[0][0].P1.Index = 0;
  DreamConfig.RMConfig[0][0].Initialised = true;
  geometry = std::make_unique<DreamGeometry>(Stats, DreamConfig);
  Readout.Anode = 16;   // Valid anode for cuboid
  Readout.Cathode = 16; // Valid cathode for cuboid
  pixel = geometry->calcPixel(Readout);
  ASSERT_TRUE(pixel >= 1122305) << "HR: Got pixel: " << pixel;
}

TEST_F(DreamGeometryTest, GetPixelError) {
  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::PA;
  Readout.UnitId = 6;
  ASSERT_EQ(geometry->calcPixel(Readout), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
