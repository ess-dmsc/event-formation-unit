// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <freia/geometry/FreiaChannelMapping.h>
#include <common/testutils/TestBase.h>

using namespace Freia;

class GeometryTest : public TestBase {
protected:
  FreiaGeometry Geom;
  uint16_t Cassette1{1};
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
    ASSERT_EQ(Geom.yCoord(Cassette1, VMMY, i), i - 16);
  }
}

TEST_F(GeometryTest, XCoordErrors) {
  ASSERT_EQ(Geom.xCoord(VMMY, 0),  Geom.InvalidCoord); // bad VMM
  ASSERT_EQ(Geom.xCoord(VMMX, 64), Geom.InvalidCoord); // bad Channel
}

TEST_F(GeometryTest, YCoordErrors) {
  ASSERT_EQ(Geom.yCoord(0, VMMY,  0), Geom.InvalidCoord); // bad cassette
  ASSERT_EQ(Geom.yCoord(1, VMMX, 32), Geom.InvalidCoord); // bad VMM
  ASSERT_EQ(Geom.yCoord(1, VMMY, 15), Geom.InvalidCoord); // bad Channel
  ASSERT_EQ(Geom.yCoord(1, VMMY, 48), Geom.InvalidCoord); // bad Channel
}

TEST_F(GeometryTest, Cassettes) {
  ASSERT_EQ(Geom.cassette(1, 0), 0); // FEN 1, VMM 0, hybrid 0
  ASSERT_EQ(Geom.cassette(1, 1), 0); // FEN 1, VMM 1
  ASSERT_EQ(Geom.cassette(1, 2), 1); // FEN 1, VMM 2, hybrid 1
  ASSERT_EQ(Geom.cassette(1, 3), 1); // FEN 1, VMM 3
  ASSERT_EQ(Geom.cassette(2, 0), 2); // FEN 2, VMM 0, hybrid 2
  ASSERT_EQ(Geom.cassette(2, 1), 2); // FEN 2, VMM 1
  ASSERT_EQ(Geom.cassette(2, 2), 3); // FEN 2, VMM 2, hybrid 3
  ASSERT_EQ(Geom.cassette(2, 3), 3); // FEN 2, VMM 3
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
