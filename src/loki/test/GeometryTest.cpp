/** Copyright (C) 2019 European Spallation Source ERIC */

#include <algorithm>
#include <memory>
#include <loki/geometry/Geometry.h>
#include <test/TestBase.h>

using namespace Loki;

class LokiGeometryTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/** Test cases below */
TEST_F(LokiGeometryTest, Constructor) {
  Geometry Geom(8, 4, 7, 512);
  ASSERT_EQ(Geom.getPixelId(), 0);
  ASSERT_EQ(Geom.getStrawId(0,0,0,0), 0);
}
