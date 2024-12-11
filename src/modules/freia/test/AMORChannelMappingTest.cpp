// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <freia/geometry/AMORChannelMapping.h>

using namespace Freia;

class AMORChannelMappingTest : public TestBase {
protected:
  AMORGeometry Geom;
  uint16_t Cassette0Xoffset{0};
  uint16_t Cassette0Yoffset{0};
  uint16_t VMMX{0};
  uint16_t VMMY{1};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(AMORChannelMappingTest, Coordinates) {
  for (unsigned int i = 0; i < 64; i++) {
    ASSERT_EQ(Geom.xCoord(Cassette0Xoffset, VMMX, i), 63 - i);
  }

  uint YCoordMinChannel = 16;
  uint YCoordMaxChannel = 47;
  uint MaxYOffset = 1024;
  uint YOffsetJumps = 32;

  for (unsigned int i = YCoordMinChannel; i < YCoordMaxChannel; i++) {
    for (unsigned int YOffset = 0; YOffset < MaxYOffset;
         YOffset += YOffsetJumps) {
      ASSERT_EQ(Geom.yCoord(YOffset, VMMY, i), YCoordMaxChannel - i + YOffset);
    }
  }
}

TEST_F(AMORChannelMappingTest, XCoordErrors) {
  ASSERT_EQ(Geom.xCoord(Cassette0Xoffset, VMMY, 0), Geom.InvalidCoord);  // bad VMM
  ASSERT_EQ(Geom.xCoord(Cassette0Xoffset, VMMX, 64), Geom.InvalidCoord); // bad Channel
}

TEST_F(AMORChannelMappingTest, YCoordErrors) {
  ASSERT_EQ(Geom.yCoord(Cassette0Yoffset, VMMX, 32), Geom.InvalidCoord); // bad VMM
  ASSERT_EQ(Geom.yCoord(Cassette0Yoffset, VMMY, 15), Geom.InvalidCoord); // bad Channel
  ASSERT_EQ(Geom.yCoord(Cassette0Yoffset, VMMY, 48), Geom.InvalidCoord); // bad Channel
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
