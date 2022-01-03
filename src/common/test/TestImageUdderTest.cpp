/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/TestImageUdder.h>
#include <common/testutils/TestBase.h>

class TestImageUdderTest : public TestBase {
protected:
  Udder UdderTestImage;
  ESSGeometry Geometry{UdderTestImage.ImageWidth, UdderTestImage.ImageHeight, 1, 1};
};

TEST_F(TestImageUdderTest, Constructor) {
  ASSERT_EQ(UdderTestImage.ImageWidth, 184); // the udder image dimension
  ASSERT_EQ(UdderTestImage.ImageHeight, 224); // the udder image dimension
  ASSERT_FALSE(UdderTestImage.isCached());
  ASSERT_EQ(UdderTestImage.getNumberOfCachedPixels(), 0);
}

TEST_F(TestImageUdderTest, FirstFewPixels) {
  auto pixel = UdderTestImage.getPixel(UdderTestImage.ImageWidth, UdderTestImage.ImageHeight, &Geometry);
  ASSERT_EQ(pixel, 123);

  pixel = UdderTestImage.getPixel(UdderTestImage.ImageWidth, UdderTestImage.ImageHeight, &Geometry);
  ASSERT_EQ(pixel, 124);
}

TEST_F(TestImageUdderTest, CachePixels) {
  ASSERT_FALSE(UdderTestImage.isCached());
  UdderTestImage.cachePixels(UdderTestImage.ImageWidth, UdderTestImage.ImageHeight, &Geometry);
  ASSERT_TRUE(UdderTestImage.isCached());
  auto pixel = UdderTestImage.getPixel(UdderTestImage.ImageWidth, UdderTestImage.ImageHeight, &Geometry);
  ASSERT_EQ(pixel, 123);

  pixel = UdderTestImage.getPixel(UdderTestImage.ImageWidth, UdderTestImage.ImageHeight, &Geometry);
  ASSERT_EQ(pixel, 124);
}

TEST_F(TestImageUdderTest, CachePixelsWrap) {
  uint32_t FirstPixel, LastPixel;
  UdderTestImage.cachePixels(UdderTestImage.ImageWidth, UdderTestImage.ImageHeight, &Geometry);
  FirstPixel = UdderTestImage.getPixel(UdderTestImage.ImageWidth, UdderTestImage.ImageHeight, &Geometry);
  for (uint32_t i = 0; i < UdderTestImage.getNumberOfCachedPixels(); i++) {
    LastPixel = UdderTestImage.getPixel(UdderTestImage.ImageWidth, UdderTestImage.ImageHeight, &Geometry);
    ASSERT_TRUE(LastPixel != 0);
  }
  ASSERT_EQ(LastPixel, 123);
  ASSERT_EQ(FirstPixel, LastPixel);
}
