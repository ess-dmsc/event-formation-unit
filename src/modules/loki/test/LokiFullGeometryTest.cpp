// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for validating Nexus Geometry against EFU Pixel values
///
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <logical_geometry/ESSGeometry.h>
#include <loki/geometry/PanelGeometry.h>
#include <memory>
#include <test/TestBase.h>
#include <vector>

using namespace Loki;

struct NexusGeometry {
  uint16_t Bank;
  uint16_t Tube;
  uint16_t Straw;
  uint16_t Pos;
  uint16_t Pixel;
};

class FullGeometryTest : public TestBase {
protected:
  const bool Vertical{true};
  const bool Horizontal{false};
  const uint32_t StrawOffset0{0};
  const uint16_t TZ4{4};      ///< # tubes in z-direction
  ESSGeometry Geometry{512, 224, 1, 1};

  std::vector<PanelGeometry *> Banks;

  void SetUp() override {
    Banks.push_back(new PanelGeometry(TZ4, 56,    0)); // Panel 0
    Banks.push_back(new PanelGeometry(TZ4, 16, 1586)); // Panel 1
    Banks.push_back(new PanelGeometry(TZ4, 12, 2016));
    Banks.push_back(new PanelGeometry(TZ4, 16, 2352));
    Banks.push_back(new PanelGeometry(TZ4, 12, 2800));
    Banks.push_back(new PanelGeometry(TZ4, 28, 3136)); // Panel 5
    Banks.push_back(new PanelGeometry(TZ4, 32, 3920));
    Banks.push_back(new PanelGeometry(TZ4, 20, 4816));
    Banks.push_back(new PanelGeometry(TZ4, 32, 5376)); // Panel 8
  }
  void TearDown() override {}
};

/// Test cases below
// From Kenan's csv file

//
TEST_F(FullGeometryTest, BasicValidation) {
  std::vector<uint32_t> fens{28, 8, 6, 8, 6, 14, 16, 10, 16};
  for (unsigned int i = 0; i < fens.size(); i++) {
    ASSERT_EQ(fens[i], Banks[i]->getMaxGroup());
  }
}

TEST_F(FullGeometryTest, FirstFewLines) {
  std::vector<struct NexusGeometry> NGData {
    {0, 1, 1,   1,    1},
    {0, 1, 1,   2,    2},
    {0, 1, 1, 511,  511},
    {0, 1, 1, 512,  512},
    {0, 1, 2,   1,  513},
    {0, 1, 2,   2,  514},
    {0, 1, 2, 511, 1023},
    {0, 1, 2, 512, 1024},
    {0, 1, 3,   1, 1025},
    {0, 1, 3,   2, 1026}
  };

  for (auto & NG : NGData) {
    uint32_t TubeGroup = (NG.Tube - 1) / 4;
    uint8_t LocalTube = (NG.Tube - 1) % 8;
    uint32_t GlobalStraw = Banks[NG.Bank]->getGlobalStrawId(TubeGroup, LocalTube, NG.Straw - 1);
    uint32_t Pixel = Geometry.pixel2D(NG.Pos - 1, GlobalStraw);
    // printf("Panel %u, Tube %u, Tube Group %u, LocTube %u, Straw %u, GblStraw %u\n",
    //   NG.Bank, NG.Tube, TubeGroup, LocalTube, NG.Straw, GlobalStraw);
    ASSERT_EQ(NG.Pixel, Pixel);
  }
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
