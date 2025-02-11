// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <freia/geometry/FreiaChannelMapping.h>

using namespace Freia;

class FreiaChannelMappingTest : public TestBase {
protected:
  FreiaGeometry Geom;
  uint16_t Cassette0Xoffset{0};
  uint16_t Cassette0Yoffset{0};
  uint16_t VMMX{1};
  uint16_t VMMY{0};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(FreiaChannelMappingTest, Coordinates) {
  for (unsigned int channel = 0; channel < 64; channel++) {
    ASSERT_EQ(Geom.xCoord(Cassette0Xoffset, VMMX, channel), 63 - channel);
  }
  for (unsigned int channel = 16; channel < 48; channel++) {
    ASSERT_EQ(Geom.yCoord(Cassette0Yoffset, VMMY, channel), 47 - channel);
  }
}

TEST_F(FreiaChannelMappingTest, XCoordErrors) {
  ASSERT_EQ(Geom.xCoord(Cassette0Xoffset, VMMY, 0), Geom.InvalidCoord);  // bad VMM
  ASSERT_EQ(Geom.xCoord(Cassette0Xoffset, VMMX, 64), Geom.InvalidCoord); // bad Channel
}

TEST_F(FreiaChannelMappingTest, YCoordErrors) {
  ASSERT_EQ(Geom.yCoord(Cassette0Yoffset, VMMX, 32), Geom.InvalidCoord); // bad VMM
  ASSERT_EQ(Geom.yCoord(Cassette0Yoffset, VMMY, 15), Geom.InvalidCoord); // bad Channel
  ASSERT_EQ(Geom.yCoord(Cassette0Yoffset, VMMY, 48), Geom.InvalidCoord); // bad Channel
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
