// Copyright (C) 2022 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <dream/geometry/DreamGeometry.h>
#include <dream/readout/DataParser.h>

using namespace Dream;

class DreamGeometryTest : public TestBase {
protected:
  DataParser::CDTReadout Readout{0, 0, 0, 0, 0, 0, 0, 0, 0};
  Config::ModuleParms Parms{false, Config::ModuleType::BwEndCap, {0}, {0}};
  DreamGeometry geometry;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DreamGeometryTest, PixelOffsets) {
  ASSERT_EQ(geometry.getPixelOffset(Config::FwEndCap), 0);
  ASSERT_EQ(geometry.getPixelOffset(Config::BwEndCap), 71680);
  ASSERT_EQ(geometry.getPixelOffset(Config::DreamMantle), 229376);
  ASSERT_EQ(geometry.getPixelOffset(Config::SANS), 720896);
  ASSERT_EQ(geometry.getPixelOffset(Config::HR), 1122304);
}

TEST_F(DreamGeometryTest, PixelOffsetsError) {
  ASSERT_EQ(geometry.getPixelOffset(Config::PA), -1);
}

TEST_F(DreamGeometryTest, GetPixel) {
  Parms.Type = Config::ModuleType::BwEndCap;
  Readout.UnitId = 6;
  ASSERT_TRUE(geometry.getPixel(Parms, Readout) >= 71681);

  Parms.Type = Config::ModuleType::DreamMantle;
  Parms.P2.Cassette = 0;
  ASSERT_TRUE(geometry.getPixel(Parms, Readout) >= 229377);

  Parms.Type = Config::ModuleType::HR;
  Parms.P2.Rotate = 0;
  ASSERT_TRUE(geometry.getPixel(Parms, Readout) >= 1122305);
}

TEST_F(DreamGeometryTest, GetPixelError) {
  Parms.Type = Config::ModuleType::PA;
  Readout.UnitId = 6;
  ASSERT_EQ(geometry.getPixel(Parms, Readout), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
