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

TEST_F(NMXGeometryTest, CoordinateCalculations) {
  // coord takes Channel, AsicId, Offset, ReversedChannels
  uint8_t AsicId = 0;
  uint16_t Offset = 0;
  bool ReversedChannels = false;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.coord(Channel, AsicId, Offset, ReversedChannels), Channel);
  }
  ReversedChannels = true;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.coord(Channel, AsicId, Offset, ReversedChannels),
              127 - Channel);
  }

  ReversedChannels = false;
  AsicId = 1;
  Offset = 512;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.coord(Channel, AsicId, Offset, ReversedChannels),
              Channel + 576);
  }
  ReversedChannels = true;
  for (int Channel = 0; Channel < 64; ++Channel) {
    ASSERT_EQ(Geom.coord(Channel, AsicId, Offset, ReversedChannels),
              575 - Channel);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
