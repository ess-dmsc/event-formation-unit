// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <nmx/geometry/NMXGeometry.h>

using namespace Nmx;

class NMXGeometryTest : public TestBase {
protected:
  NMXGeometry Geom;
  uint16_t VMM0{0};
  uint16_t VMM1{1};
  uint16_t VMM2{2};

  void SetUp() override {}
  void TearDown() override {}
};

// Should match the ICD
TEST_F(NMXGeometryTest, DefaultNMX) {
  
}

TEST_F(NMXGeometryTest, CoordinateCalculations) {
  // coord takes FENID, HybridID, VMMID, Channel, XOffset, Rotated
}

TEST_F(NMXGeometryTest, InvalidCoordinates) {
  
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
