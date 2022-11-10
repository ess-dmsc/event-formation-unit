// Copyright (C) 2021-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit tests for validating Nexus Geometry against EFU Pixel values
///
//===----------------------------------------------------------------------===//

#include <LokiFullGeometryTestData.h>
#include <algorithm>
#include <loki/geometry/PanelGeometry.h>
#include <common/testutils/TestBase.h>
#include <cstdint>
#include <logical_geometry/ESSGeometry.h>
#include <memory>
#include <vector>

using namespace Caen;

class FullGeometryTest : public TestBase {
protected:
  const bool Vertical{true};
  const bool Horizontal{false};
  const uint32_t StrawOffset0{0};
  const uint16_t TZ4{4}; ///< # tubes in z-direction
  ESSGeometry Geometry{512, 6272, 1, 1};

  std::vector<PanelGeometry *> Banks;

  void SetUp() override {
    Banks.push_back(new PanelGeometry(TZ4, 56, 0));    // Panel 0
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

// Compare pixelids used by the generated nexux files
// https://github.com/ess-dmsc/generate-nexus-files
// There is a slight difference in interpretation of TubeIDs, but this is not
// essential so here we just convert one counting convention into another.
TEST_F(FullGeometryTest, FirstFewLines) {

  for (auto &NG : NGData) {
    // helper values
    uint8_t NBTubes = Banks[NG.Bank]->getMaxGroup() * 2; // In non-Z direction
    uint8_t Column = NG.Tube / NBTubes;
    uint8_t Row = NG.Tube % 2;

    uint32_t TubeGroup = (NG.Tube % NBTubes) / 2;
    uint8_t LocalTube = Row * 4 + Column;
    uint32_t GlobalStraw =
        Banks[NG.Bank]->getGlobalStrawId(TubeGroup, LocalTube, NG.Straw % 7);
    uint32_t Pixel = Geometry.pixel2D(NG.Pos, GlobalStraw);

    // printf("tube %u, tubegroup %u, loctube %u, locstraw %u, col %u, row %u,
    // gblstraw %u, pixel %u\n",
    //   NG.Tube, TubeGroup, LocalTube, NG.Straw % 7, Column, Row, GlobalStraw,
    //   Pixel);
    ASSERT_EQ(NG.Pixel, Pixel);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
