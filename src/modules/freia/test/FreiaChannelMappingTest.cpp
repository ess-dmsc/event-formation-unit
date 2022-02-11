// Copyright (C) 2021 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <freia/geometry/FreiaChannelMapping.h>

using namespace Freia;

class GeometryTest : public TestBase {
protected:
  FreiaGeometry Geom;
  uint16_t Cassette0{0};
  uint16_t VMMX{1};
  uint16_t VMMY{0};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(GeometryTest, Coordinates) {
  for (unsigned int i = 0; i < 64; i++) {
    ASSERT_EQ(Geom.xCoord(VMMX, i), i);
  }
  for (unsigned int i = 16; i < 47; i++) {
    ASSERT_EQ(Geom.yCoord(Cassette0, VMMY, i), i - 16);
  }
}

TEST_F(GeometryTest, XCoordErrors) {
  ASSERT_EQ(Geom.xCoord(VMMY, 0), Geom.InvalidCoord);  // bad VMM
  ASSERT_EQ(Geom.xCoord(VMMX, 64), Geom.InvalidCoord); // bad Channel
}

TEST_F(GeometryTest, YCoordErrors) {
  ASSERT_EQ(Geom.yCoord(1, VMMX, 32), Geom.InvalidCoord); // bad VMM
  ASSERT_EQ(Geom.yCoord(1, VMMY, 15), Geom.InvalidCoord); // bad Channel
  ASSERT_EQ(Geom.yCoord(1, VMMY, 48), Geom.InvalidCoord); // bad Channel
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
