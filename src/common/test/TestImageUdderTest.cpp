/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/TestImageUdder.h>
#include <test/TestBase.h>

class TestImageUdderTest : public TestBase {
protected:
  Udder udder;
  ESSGeometry geom{udder.ImageWidth, udder.ImageHeight, 1, 1};
};

TEST_F(TestImageUdderTest, Constructor) {
  ASSERT_EQ(udder.ImageWidth, 184); // the udder image dimension
  ASSERT_EQ(udder.ImageHeight, 224); // the udder image dimension
  ASSERT_FALSE(udder.isCached());
  ASSERT_EQ(udder.getNumberOfCachedPixels(), 0);
}

TEST_F(TestImageUdderTest, FirstFewPixels) {
  auto pixel = udder.getPixel(udder.ImageWidth, udder.ImageHeight, &geom);
  ASSERT_EQ(pixel, 123);

  pixel = udder.getPixel(udder.ImageWidth, udder.ImageHeight, &geom);
  ASSERT_EQ(pixel, 124);
}

TEST_F(TestImageUdderTest, CachePixels) {
  ASSERT_FALSE(udder.isCached());
  udder.cachePixels(udder.ImageWidth, udder.ImageHeight, &geom);
  ASSERT_TRUE(udder.isCached());
  auto pixel = udder.getPixel(udder.ImageWidth, udder.ImageHeight, &geom);
  ASSERT_EQ(pixel, 123);

  pixel = udder.getPixel(udder.ImageWidth, udder.ImageHeight, &geom);
  ASSERT_EQ(pixel, 124);
}

TEST_F(TestImageUdderTest, CachePixelsWrap) {
  uint32_t FirstPixel, LastPixel;
  udder.cachePixels(udder.ImageWidth, udder.ImageHeight, &geom);
  FirstPixel = udder.getPixel(udder.ImageWidth, udder.ImageHeight, &geom);
  for (uint32_t i = 0; i < udder.getNumberOfCachedPixels(); i++) {
    LastPixel = udder.getPixel(udder.ImageWidth, udder.ImageHeight, &geom);
    ASSERT_TRUE(LastPixel != 0);
  }
  ASSERT_EQ(LastPixel, 123);
  ASSERT_EQ(FirstPixel, LastPixel);
}
