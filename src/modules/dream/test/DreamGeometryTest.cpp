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
  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::BwEndCap;
  Readout.UnitId = 6;
  ASSERT_TRUE(geometry->calcPixel(Readout) >= 71681);

  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::DreamMantle;
  DreamConfig.RMConfig[0][0].P2.Cassette = 0;
  ASSERT_TRUE(geometry->calcPixel(Readout) >= 229377);

  DreamConfig.RMConfig[0][0].Type = Config::ModuleType::HR;
  DreamConfig.RMConfig[0][0].P2.Rotate = 0;
  ASSERT_TRUE(geometry->calcPixel(Readout) >= 1122305);
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
