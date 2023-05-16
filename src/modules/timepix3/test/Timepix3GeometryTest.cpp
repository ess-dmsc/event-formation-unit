// Copyright (C) 2020-2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <timepix3/geometry/Geometry.h>
#include <modules/timepix3/readout/DataParser.h>
#include <logical_geometry/ESSGeometry.h>




using namespace Timepix3;

class Timepix3GeometryTest : public TestBase {
protected:
  struct Counters counters;

  Geometry *timepix3geom;

  void SetUp() override {
     timepix3geom = new Geometry();
     timepix3geom->ESSGeom = new ESSGeometry(256, 256, 1, 1);
   }
  void TearDown() override {}
};

// Test cases below
TEST_F(Timepix3GeometryTest, DefaultTimepix3) {
  DataParser::Timepix3PixelReadout Data = {0, 0, 0, 0, 0, 0, 0};
  EXPECT_EQ(timepix3geom->calcPixel(Data), 1);
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}
