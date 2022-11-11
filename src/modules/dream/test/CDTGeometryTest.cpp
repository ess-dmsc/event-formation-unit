// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <dream/geometry/CDTGeometry.h>
#include <dream/readout/DataParser.h>

using namespace Dream;

class CDTGeometryTest : public TestBase {
protected:
  DataParser::DreamReadout Readout{0, 0, 0, 0, 0, 0, 0, 0, 0};
  Config::ModuleParms Parms{false, Config::ModuleType::BwEndCap, {0}, {0}};
  CDTGeometry geometry;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(CDTGeometryTest, PixelOffsets) {
  ASSERT_EQ(geometry.getPixelOffset(Config::FwEndCap), 0);
  ASSERT_EQ(geometry.getPixelOffset(Config::BwEndCap), 71680);
  ASSERT_EQ(geometry.getPixelOffset(Config::Mantle), 229376);
  ASSERT_EQ(geometry.getPixelOffset(Config::SANS), 720896);
  ASSERT_EQ(geometry.getPixelOffset(Config::HR), 1122304);
}

TEST_F(CDTGeometryTest, GetPixel) {
  Parms.Type = Config::ModuleType::BwEndCap;
  Parms.P2.SumoPair = 6;
  ASSERT_TRUE(geometry.getPixel(Parms, Readout) >= 71681);

  Parms.Type = Config::ModuleType::Mantle;
  Parms.P2.Cassette = 0;
  ASSERT_TRUE(geometry.getPixel(Parms, Readout) >= 229377);

  Parms.Type = Config::ModuleType::HR;
  Parms.P2.Rotate = 0;
  ASSERT_TRUE(geometry.getPixel(Parms, Readout) >= 1122305);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
