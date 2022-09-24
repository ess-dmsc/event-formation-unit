// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <dream/geometry/CDTGeometry.h>

using namespace Dream;

class CDTGeometryTest : public TestBase {
protected:
  CDTGeometry geometry;
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(CDTGeometryTest, PixelOffsets) {
  ASSERT_EQ(geometry.getPixelOffset(Config::FwEndCap), 0);
  ASSERT_EQ(geometry.getPixelOffset(Config::BwEndCap), 71680);
  ASSERT_EQ(geometry.getPixelOffset(Config::Mantle), 229376);
  ASSERT_EQ(geometry.getPixelOffset(Config::SANS), 720896);
  ASSERT_EQ(geometry.getPixelOffset(Config::HR), 1122304);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
