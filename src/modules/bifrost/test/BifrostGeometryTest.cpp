
// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Bifrost position calculations
///
//===----------------------------------------------------------------------===//
#include <bifrost/geometry/Geometry.h>
#include <common/testutils/TestBase.h>

using namespace Bifrost;

class BifrostGeometryTest : public TestBase {
protected:
  Geometry geom;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(BifrostGeometryTest, YOffset) {
  ASSERT_EQ(geom.yOffset(0), 0);
  ASSERT_EQ(geom.yOffset(1), 0);
  ASSERT_EQ(geom.yOffset(2), 0);
  ASSERT_EQ(geom.yOffset(3), 3);
  ASSERT_EQ(geom.yOffset(4), 3);
  ASSERT_EQ(geom.yOffset(5), 3);
  ASSERT_EQ(geom.yOffset(6), 6);
  ASSERT_EQ(geom.yOffset(7), 6);
  ASSERT_EQ(geom.yOffset(8), 6);
  ASSERT_EQ(geom.yOffset(9), 9);
  ASSERT_EQ(geom.yOffset(10), 9);
  ASSERT_EQ(geom.yOffset(11), 9);
  ASSERT_EQ(geom.yOffset(12), 12);
  ASSERT_EQ(geom.yOffset(13), 12);
  ASSERT_EQ(geom.yOffset(14), 12);
}

TEST_F(BifrostGeometryTest, XOffset) {
  ASSERT_EQ(geom.xOffset(0, 0), 0);
  ASSERT_EQ(geom.xOffset(0, 1), 100);
  ASSERT_EQ(geom.xOffset(0, 2), 200);
  ASSERT_EQ(geom.xOffset(1, 0), 300);
  ASSERT_EQ(geom.xOffset(1, 1), 400);
  ASSERT_EQ(geom.xOffset(1, 2), 500);
  ASSERT_EQ(geom.xOffset(2, 0), 600);
  ASSERT_EQ(geom.xOffset(2, 1), 700);
  ASSERT_EQ(geom.xOffset(2, 2), 800);
}

TEST_F(BifrostGeometryTest, Position) {
  ASSERT_EQ(geom.posAlongTube(0, 0), -1);
  ASSERT_EQ(geom.posAlongTube(0, 1), 0);
  ASSERT_EQ(geom.posAlongTube(1, 0), 299);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
