// Copyright (C) 2019-2023 European Spallation Source ERIC

#include <algorithm>
#include <common/testutils/TestBase.h>
#include <loki/geometry/PanelGeometry.h>
#include <memory>

using namespace Caen;

class PanelGeometryTest : public TestBase {
protected:
  const bool Vertical{true};
  const bool Horizontal{false};
  const uint32_t UnitOffset0{0};
  const uint16_t GroupssXY8{8}; ///< # tubes in x or y direction
  const uint16_t TZ4{4};        ///< # tubes in z-direction
  void SetUp() override {}
  void TearDown() override {}
};

// Test cases below
TEST_F(PanelGeometryTest, Constructor) {
  PanelGeometry PG(TZ4, GroupssXY8, UnitOffset0);
  const uint8_t GroupBank{0};
  const uint8_t Group{0};
  const uint8_t Unit{0};
  uint32_t GlobalUnit = PG.getGlobalUnitId(GroupBank, Group, Unit);
  ASSERT_EQ(GlobalUnit, 0);
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
  PanelGeometry PG(TZ4, GroupssXY8, UnitOffset0);
  const int UnitsPerCol{GroupssXY8 * 7};
  ASSERT_EQ(PG.getGlobalUnitId(0, 0, 0), 0 * UnitsPerCol);     // Top left
  ASSERT_EQ(PG.getGlobalUnitId(0, 3, 0), 3 * UnitsPerCol);     // Top right
  ASSERT_EQ(PG.getGlobalUnitId(3, 4, 6), UnitsPerCol - 1);     // Bottom left
  ASSERT_EQ(PG.getGlobalUnitId(3, 7, 6), 4 * UnitsPerCol - 1); // Bottom right
}

TEST_F(PanelGeometryTest, CanonicalCornersWithOffset) {
  const uint32_t Offset10000{10000};
  PanelGeometry PG(TZ4, GroupssXY8, Offset10000);
  const int UnitsPerCol{GroupssXY8 * 7};
  ASSERT_EQ(PG.getGlobalUnitId(0, 0, 0),
            0 * UnitsPerCol + Offset10000); // Top left
  ASSERT_EQ(PG.getGlobalUnitId(0, 3, 0),
            3 * UnitsPerCol + Offset10000); // Top right
  ASSERT_EQ(PG.getGlobalUnitId(3, 4, 6),
            UnitsPerCol - 1 + Offset10000); // Bottom left
  ASSERT_EQ(PG.getGlobalUnitId(3, 7, 6),
            4 * UnitsPerCol - 1 + Offset10000); // Bottom right
}

TEST_F(PanelGeometryTest, InvalidGeometry) {
  PanelGeometry PG(TZ4, GroupssXY8, UnitOffset0);
  ASSERT_NE(PG.getGlobalUnitId(3, 0, 0), 0xffffffff); // valid geometry
  ASSERT_EQ(PG.getGlobalUnitId(4, 0, 0), 0xffffffff); // Tube group too large
  ASSERT_NE(PG.getGlobalUnitId(2, 7, 0), 0xffffffff); // valid geometry
  ASSERT_EQ(PG.getGlobalUnitId(2, 8, 0), 0xffffffff); // Local tube too large
  ASSERT_NE(PG.getGlobalUnitId(1, 0, 6), 0xffffffff); // valid geometry
  ASSERT_EQ(PG.getGlobalUnitId(1, 0, 7), 0xffffffff); // Unit too large
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
