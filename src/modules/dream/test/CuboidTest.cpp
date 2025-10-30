// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <memory>
#include <dream/geometry/Cuboid.h>
#include <dream/readout/DataParser.h>

using namespace Dream;

class CuboidGeometryTest : public TestBase {
protected:
  Statistics Stats;
  DataParser::CDTReadout Readout{0, 0, 0, 0, 0, 0, 0, 0, 0};
  Config::ModuleParms Parms{false, Config::ModuleType::HR, {0}, {0}};
  std::unique_ptr<Cuboid> CuboidGeometry;

  void SetUp() override {
    CuboidGeometry = std::make_unique<Cuboid>(Stats);
  }
};

TEST_F(CuboidGeometryTest, IndexSizes) {
  ASSERT_EQ(CuboidGeometry->OffsetsHR.size(), CuboidGeometry->RotateHR.size());
  ASSERT_EQ(CuboidGeometry->OffsetsSANS.size(), CuboidGeometry->RotateSANS.size());
}

TEST_F(CuboidGeometryTest, Rotate0) {
  int x = 1;
  int y = 12;
  CuboidGeometry->rotateXY(x, y, 0);
  ASSERT_EQ(x, 1);
  ASSERT_EQ(y, 12);
}

TEST_F(CuboidGeometryTest, Rotate1) {
  int x = 1;
  int y = 12;
  CuboidGeometry->rotateXY(x, y, 1);
  ASSERT_NE(x, 1);
  ASSERT_NE(y, 12);
  CuboidGeometry->rotateXY(x, y, 1);
  CuboidGeometry->rotateXY(x, y, 1);
  CuboidGeometry->rotateXY(x, y, 1);
  ASSERT_EQ(x, 1);
  ASSERT_EQ(y, 12);
}

TEST_F(CuboidGeometryTest, Rotate2) {
  int x = 1;
  int y = 12;
  CuboidGeometry->rotateXY(x, y, 2);
  ASSERT_NE(x, 1);
  ASSERT_NE(y, 12);
  CuboidGeometry->rotateXY(x, y, 2);
  ASSERT_EQ(x, 1);
  ASSERT_EQ(y, 12);
}

TEST_F(CuboidGeometryTest, Rotate3) {
  int x = 1;
  int y = 12;
  CuboidGeometry->rotateXY(x, y, 3);
  ASSERT_NE(x, 1);
  ASSERT_NE(y, 12);
  CuboidGeometry->rotateXY(x, y, 1);
  ASSERT_EQ(x, 1);
  ASSERT_EQ(y, 12);
}

TEST_F(CuboidGeometryTest, OffsetRange) {
  Parms.P1.Index = 32;
  ASSERT_NE(CuboidGeometry->calcPixelId(Parms, Readout), 0);
  Parms.P1.Index = 33;
  ASSERT_EQ(CuboidGeometry->calcPixelId(Parms, Readout), 0);

  auto counters = CuboidGeometry->getCuboidCounters();
  ASSERT_EQ(counters.IndexErrors, 1);
  ASSERT_EQ(counters.TypeErrors, 0);

  Parms.Type = Config::ModuleType::SANS;
  Parms.P1.Index = 35;
  ASSERT_NE(CuboidGeometry->calcPixelId(Parms, Readout), 0);
  Parms.P1.Index = 36;
  ASSERT_EQ(CuboidGeometry->calcPixelId(Parms, Readout), 0);

  counters = CuboidGeometry->getCuboidCounters();
  ASSERT_EQ(counters.IndexErrors, 2);
  ASSERT_EQ(counters.TypeErrors, 0);
}

TEST_F(CuboidGeometryTest, InvalidModuleType) {
  Parms.Type = Config::ModuleType::FwEndCap;
  Parms.P1.Index = 0;
  ASSERT_EQ(CuboidGeometry->calcPixelId(Parms, Readout), 0);

  const auto &counters = CuboidGeometry->getCuboidCounters();
  ASSERT_EQ(counters.IndexErrors, 0);
  ASSERT_EQ(counters.TypeErrors, 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
