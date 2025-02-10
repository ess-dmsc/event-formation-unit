// Copyright (C) 2023 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <common/testutils/TestBase.h>
#include <cstdint>
#include <dto/TimepixDataTypes.h>
#include <gtest/gtest.h>
#include <handlers/PixelEventHandler.h>
#include <logical_geometry/ESSGeometry.h>
#include <memory>
#include <modules/timepix3/readout/DataParser.h>
#include <timepix3/geometry/Timepix3Geometry.h>

using namespace Timepix3;
using namespace timepixReadout;
using namespace std;

class Timepix3GeometryTest : public TestBase {
protected:
  std::unique_ptr<Timepix3Geometry> timepix3geom;

  void SetUp() override {
    timepix3geom = make_unique<Timepix3Geometry>(256, 256, 1, 1);
  }

  void TearDown() override {}
};

// Test cases below
TEST_F(Timepix3GeometryTest, calculatePixelIdFor0) {
  PixelReadout Data = {0, 0, 0, 0, 0, 0, 0};
  uint32_t XCoord = timepix3geom->calcX(Data);
  uint32_t YCoord = timepix3geom->calcY(Data);
  EXPECT_EQ(XCoord, static_cast<uint32_t>(0));
  EXPECT_EQ(YCoord, static_cast<uint32_t>(0));

  EXPECT_EQ(timepix3geom->calcPixelId(XCoord, YCoord),
            static_cast<uint32_t>(1));
}

TEST_F(Timepix3GeometryTest, calculatePixelIdForInternPixel) {
  PixelReadout Data = {200, 10, 11, 0, 0, 0, 0};
  uint32_t XCoord = timepix3geom->calcX(Data);
  uint32_t YCoord = timepix3geom->calcY(Data);
  EXPECT_EQ(XCoord, static_cast<uint32_t>(202));
  EXPECT_EQ(YCoord, static_cast<uint32_t>(13));
  EXPECT_EQ(timepix3geom->calcPixelId(XCoord, YCoord),
            static_cast<uint32_t>(3531));
}

TEST_F(Timepix3GeometryTest, TestUpScaledGeometry) {
  PixelReadout Data = {10, 10, 0, 0, 0, 0, 0};

  uint8_t upscaleFactor = 2;

  Timepix3Geometry UpSampledTimepix3Geom(256, 256, upscaleFactor, 1);
  uint32_t XCoord = UpSampledTimepix3Geom.calcX(Data);
  uint32_t YCoord = UpSampledTimepix3Geom.calcY(Data);

  EXPECT_EQ(XCoord, static_cast<uint32_t>(10));
  EXPECT_EQ(YCoord, static_cast<uint32_t>(10));
  EXPECT_EQ(UpSampledTimepix3Geom.calcPixelId(XCoord, YCoord),
            static_cast<uint32_t>(upscaleFactor * YCoord *
                                      UpSampledTimepix3Geom.nx() +
                                  upscaleFactor * XCoord + 1));

  upscaleFactor = 4;

  UpSampledTimepix3Geom = Timepix3Geometry(256, 256, upscaleFactor, 1);
  XCoord = UpSampledTimepix3Geom.calcX(Data);
  YCoord = UpSampledTimepix3Geom.calcY(Data);

  EXPECT_EQ(XCoord, static_cast<uint32_t>(10));
  EXPECT_EQ(YCoord, static_cast<uint32_t>(10));
  EXPECT_EQ(UpSampledTimepix3Geom.calcPixelId(XCoord, YCoord),
            static_cast<uint32_t>(upscaleFactor * YCoord *
                                      UpSampledTimepix3Geom.nx() +
                                  upscaleFactor * XCoord + 1));
}

TEST_F(Timepix3GeometryTest, ValidateionOfPixelData) {
  PixelReadout columnOutOfBounds = {256, 0, 0, 0, 0, 0, 0};
  EXPECT_FALSE(timepix3geom->validateData(columnOutOfBounds));
  PixelReadout rowOutOfBounds = {0, 256, 0, 0, 0, 0, 0};
  EXPECT_FALSE(timepix3geom->validateData(rowOutOfBounds));
  PixelReadout withinLimits = {255, 254, 1, 0, 0, 0, 0};
  EXPECT_TRUE(timepix3geom->validateData(withinLimits));
}

TEST_F(Timepix3GeometryTest, chukWindowsIndexfor1Chunk) {
  EXPECT_EQ(timepix3geom->getChunkWindowIndex(1, 1), 0);
  EXPECT_EQ(timepix3geom->getChunkWindowIndex(256, 256), 0);
  EXPECT_EQ(timepix3geom->getChunkWindowIndex(256, 1), 0);
  EXPECT_EQ(timepix3geom->getChunkWindowIndex(1, 256), 0);
}

TEST_F(Timepix3GeometryTest, chukWindowsIndexfor4Chunks) {
  Timepix3Geometry testGeaom(256, 256, 1, 4);
  EXPECT_EQ(testGeaom.getChunkNumber(), 4);
  EXPECT_EQ(testGeaom.getChunkWindowIndex(0, 0), 0);
  EXPECT_EQ(testGeaom.getChunkWindowIndex(255, 1), 1);
  EXPECT_EQ(testGeaom.getChunkWindowIndex(1, 255), 2);
  EXPECT_EQ(testGeaom.getChunkWindowIndex(255, 255), 3);

  Timepix3Geometry testGeaom2(40, 40, 1, 16);
  EXPECT_EQ(testGeaom2.getChunkNumber(), 16);
  EXPECT_EQ(testGeaom2.getChunkWindowIndex(1, 1), 0);
  EXPECT_EQ(testGeaom2.getChunkWindowIndex(11, 1), 1);
  EXPECT_EQ(testGeaom2.getChunkWindowIndex(21, 1), 2);
  EXPECT_EQ(testGeaom2.getChunkWindowIndex(31, 1), 3);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  auto RetVal = RUN_ALL_TESTS();

  return RetVal;
}
