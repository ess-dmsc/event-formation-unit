


// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Unit test for Miracles position calculations
///
//===----------------------------------------------------------------------===//
#include <caen/geometry/MiraclesGeometry.h>
#include <common/testutils/TestBase.h>

using namespace Caen;

class MiraclesGeometryTest : public TestBase {
protected:
  MiraclesGeometry* geom;
  Config CaenConfiguration;
  void SetUp() override {
    geom = new MiraclesGeometry(CaenConfiguration);
    geom->NPos = 128;
  }
  void TearDown() override {}
};


TEST_F(MiraclesGeometryTest, Corner) {
  ASSERT_EQ( 0, geom->xCoord(0, 0, 0, 5));
  ASSERT_EQ( 0, geom->yCoord(0, 0, 5));

  ASSERT_EQ(47, geom->xCoord(1, 11, 5, 0));
  ASSERT_EQ( 0, geom->yCoord(1, 5, 0));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}