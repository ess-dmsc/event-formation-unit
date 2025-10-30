// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/Statistics.h>
#include <common/testutils/TestBase.h>
#include <dream/geometry/Config.h>
#include <dream/geometry/MagicGeometry.h>
#include <dream/readout/DataParser.h>
#include <memory>

using namespace Dream;

class MagicGeometryTest : public TestBase {
protected:
  DataParser::CDTReadout Readout{0, 0, 0, 0, 0, 0, 0, 0, 0};
  std::unique_ptr<MagicGeometry> geometry;
  Statistics Stats;
  Config DreamConfig;

  MagicGeometryTest() = default;

  void SetUp() override {
    // Create geometry with config
    geometry = std::make_unique<MagicGeometry>(Stats, DreamConfig);
  }

  void TearDown() override {}
};

TEST_F(MagicGeometryTest, PixelOffsets) {
  ASSERT_EQ(geometry->getPixelOffset(Config::FR), 0);
  ASSERT_EQ(geometry->getPixelOffset(Config::PA), 245760);
  ASSERT_EQ(geometry->getPixelOffset(Config::HR), -1);
}

TEST_F(MagicGeometryTest, GetPixel) {
  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::BwEndCap;
  ASSERT_EQ(geometry->calcPixel<DataParser::CDTReadout>(Readout), 0);

  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::FR;
  ASSERT_TRUE(geometry->calcPixel<DataParser::CDTReadout>(Readout) >= 1);
  ASSERT_TRUE(geometry->calcPixel<DataParser::CDTReadout>(Readout) < 245761);

  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::PA;
  ASSERT_TRUE(geometry->calcPixel<DataParser::CDTReadout>(Readout) >= 245761);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
