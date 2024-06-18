// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file

#include <tbl3he/geometry/Tbl3HeGeometry.h>
#include <common/testutils/TestBase.h>

using namespace Caen;


class Tbl3HeGeometryTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(Tbl3HeGeometryTest, Constructor) {
  EXPECT_TRUE(true);
}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
