/** Copyright (C) 2019-2022 European Spallation Source ERIC */

#include <algorithm>
#include <loki/geometry/PanelGeometry.h>
#include <common/testutils/TestBase.h>
#include <memory>

using namespace Caen;

class PanelGeometryTest : public TestBase {
protected:
  const bool Vertical{true};
  const bool Horizontal{false};
  const uint32_t StrawOffset0{0};
  const uint16_t TubesXY8{8}; ///< # tubes in x or y direction
  const uint16_t TZ4{4};      ///< # tubes in z-direction
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(PanelGeometryTest, Constructor) {
  PanelGeometry PG(TZ4, TubesXY8, StrawOffset0);
  const uint8_t TubeGroup{0};
  const uint8_t LocalTube{0};
  const uint8_t LocalStraw{0};
  uint32_t Straw = PG.getGlobalStrawId(TubeGroup, LocalTube, LocalStraw);
  ASSERT_EQ(Straw, 0);
}

/// Group0  (0) (1) (2) (3)
///         (4) (5) (6) (7)
/// Group1  (0) (1) (2) (3)
///         (4) (5) (6) (7)
/// Group2  (0) (1) (2) (3)
///         (4) (5) (6) (7)
/// Group3  (0) (1) (2) (3)
///         (4) (5) (6) (7)
TEST_F(PanelGeometryTest, CanonicalCorners) {
  PanelGeometry PG(TZ4, TubesXY8, StrawOffset0);
  const int StrawsPerCol{TubesXY8 * 7};
  ASSERT_EQ(PG.getGlobalStrawId(0, 0, 0), 0 * StrawsPerCol);     // Top left
  ASSERT_EQ(PG.getGlobalStrawId(0, 3, 0), 3 * StrawsPerCol);     // Top right
  ASSERT_EQ(PG.getGlobalStrawId(3, 4, 6), StrawsPerCol - 1);     // Bottom left
  ASSERT_EQ(PG.getGlobalStrawId(3, 7, 6), 4 * StrawsPerCol - 1); // Bottom right
}

TEST_F(PanelGeometryTest, CanonicalCornersWithOffset) {
  const uint32_t Offset10000{10000};
  PanelGeometry PG(TZ4, TubesXY8, Offset10000);
  const int StrawsPerCol{TubesXY8 * 7};
  ASSERT_EQ(PG.getGlobalStrawId(0, 0, 0),
            0 * StrawsPerCol + Offset10000); // Top left
  ASSERT_EQ(PG.getGlobalStrawId(0, 3, 0),
            3 * StrawsPerCol + Offset10000); // Top right
  ASSERT_EQ(PG.getGlobalStrawId(3, 4, 6),
            StrawsPerCol - 1 + Offset10000); // Bottom left
  ASSERT_EQ(PG.getGlobalStrawId(3, 7, 6),
            4 * StrawsPerCol - 1 + Offset10000); // Bottom right
}

TEST_F(PanelGeometryTest, InvalidGeometry) {
  PanelGeometry PG(TZ4, TubesXY8, StrawOffset0);
  ASSERT_NE(PG.getGlobalStrawId(3, 0, 0), 0xffffffff); // valid geometry
  ASSERT_EQ(PG.getGlobalStrawId(4, 0, 0), 0xffffffff); // Tube group too large
  ASSERT_NE(PG.getGlobalStrawId(2, 7, 0), 0xffffffff); // valid geometry
  ASSERT_EQ(PG.getGlobalStrawId(2, 8, 0), 0xffffffff); // Local tube too large
  ASSERT_NE(PG.getGlobalStrawId(1, 0, 6), 0xffffffff); // valid geometry
  ASSERT_EQ(PG.getGlobalStrawId(1, 0, 7), 0xffffffff); // Straw too large
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
