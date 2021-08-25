// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <freia/geometry/Geometry.h>
#include <test/SaveBuffer.h>
#include <test/TestBase.h>


using namespace Freia;

class GeometryTest : public TestBase {
protected:
  Geometry Geom;
  uint16_t Cassette1{1};
  uint16_t VMM1{1};
  uint16_t VMM0{0};
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(GeometryTest, Constructor) {
  ASSERT_EQ(Geom.xCoord(           VMM1, 0), 0); // VMM 1 == x
  ASSERT_EQ(Geom.yCoord(Cassette1, VMM0, Geom.MinWireChannel), 0); // VMM 0 == y
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
