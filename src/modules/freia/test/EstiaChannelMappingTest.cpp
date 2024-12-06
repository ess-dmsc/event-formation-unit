// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <freia/geometry/EstiaChannelMapping.h>

using namespace Freia;

class EstiaChannelMappingTest : public TestBase {
protected:
  EstiaGeometry Geom;
  uint16_t Cassette0XOffset{0};
  uint16_t Cassette0YOffset{0};
  uint16_t VMMX{1};
  uint16_t VMMY{0};
  void SetUp() override {}
  void TearDown() override {}
};



TEST_F(EstiaChannelMappingTest, Coordinates) {
  for (unsigned int Channel = 16; Channel < 48; Channel++) {
    unsigned int xCoord = Channel - 16; // Channel offset for wires
    ASSERT_EQ(Geom.xCoord(Cassette0XOffset, VMMX, Channel), xCoord);
  }

  for (unsigned int Channel = 0; Channel < 64; Channel++) {
    unsigned int yCoord = 63 - Channel;
    ASSERT_EQ(Geom.yCoord(Cassette0YOffset, VMMY, Channel), yCoord);
  }
}

TEST_F(EstiaChannelMappingTest, XCoordErrors) {
  ASSERT_EQ(Geom.xCoord(Cassette0XOffset, VMMY, 32), Geom.InvalidCoord);  // bad VMM
  ASSERT_EQ(Geom.xCoord(Cassette0XOffset, VMMX, 15), Geom.InvalidCoord); // bad Channel
  ASSERT_EQ(Geom.xCoord(Cassette0XOffset, VMMX, 48), Geom.InvalidCoord); // bad Channel
}

TEST_F(EstiaChannelMappingTest, YCoordErrors) {
  ASSERT_EQ(Geom.yCoord(Cassette0YOffset, VMMX, 32), Geom.InvalidCoord); // bad VMM
  ASSERT_EQ(Geom.yCoord(Cassette0YOffset, VMMY, 64), Geom.InvalidCoord); // bad Channel
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
