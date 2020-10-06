/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/Config.h>
#include <test/TestBase.h>

#include <common/Assert.h>

class DreamIcdTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/// \todo Handle pixel min/max value test?
/// \todo Benchmark signed/unsigned int. Unsigned looks much shorter on godbolt.

// test global pixel to sumo slice mapping

struct SliceInfo {
  int SectorIdx;
  int StripIdx;
  int SliceColIdx;
  int SliceRowIdx;
};

enum SliceMapConstants {
  SliceWidth =
      56, /// Width of the Sector Sumo slice in logical coordinates, in pixels.
  SliceHeight =
      16, /// Width of the Sector Sumo slice in logical coordinates, in pixels.
  SectorCount =
      23, /// Number of Sectors, left to right, in the logical coordinates.
  TotalWidth = SliceWidth * SectorCount
};

/// \brief this maps pixelid to the SectorStripSlice. SliceRowIdx and
/// SliceColIdx are coordinates inside the slice.
/// \todo can this be changed to "masking" by doing PixelIdFromSliceInfo() in
/// "reverse"?
SliceInfo PixelIdToSliceInfo(int PixelId) {
  SliceInfo Info;
  int PixelIdx = PixelId - 1;
  int SectorIdx = PixelIdx / SliceWidth;
  int GlobalRowIdx = PixelIdx / TotalWidth;
  Info.SectorIdx = SectorIdx % SectorCount;
  Info.StripIdx = GlobalRowIdx / SliceHeight;
  Info.SliceColIdx = PixelIdx % SliceWidth;
  Info.SliceRowIdx = GlobalRowIdx % SliceHeight;
  return Info;
}

int PixelIdFromSliceInfo(SliceInfo Info) {
  int PixelId = 1 + Info.SliceColIdx + Info.SectorIdx * (SliceWidth) +
                Info.SliceRowIdx * (SliceWidth * SectorCount) +
                Info.StripIdx * (SliceWidth * SectorCount * SliceHeight);
  return PixelId;
}

int SumoStartColOffsetFromSliceCol(int SliceColIdx){
  // clang-format off
  static const int SumoStartColOffset[14] = {
     0,  0,  0,  0,  0, // 0-4
    20, 20, 20, 20,     // 5-8
    36, 36, 36,         // 9-11
    48, 48,             // 12-13
  }; 
  // clang-format on
  int SliceColDiv4 = SliceColIdx / 4;
  RelAssertMsg(SliceColDiv4 < 14, "Bad SliceColIdx");
  int SumoStartCol = SumoStartColOffset[SliceColDiv4];
  return SumoStartCol;
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_Pixel1) {
  int PixelId = 1;
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 0);
  ASSERT_EQ(Info.StripIdx, 0);
  ASSERT_EQ(Info.SliceColIdx, 0);
  ASSERT_EQ(Info.SliceRowIdx, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_Sector2Pixel1) {
  int PixelId = 1 + SliceWidth;
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 1);
  ASSERT_EQ(Info.StripIdx, 0);
  ASSERT_EQ(Info.SliceColIdx, 0);
  ASSERT_EQ(Info.SliceRowIdx, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_Sector3Pixel1) {
  int PixelId = 1 + 2 * SliceWidth;
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 2);
  ASSERT_EQ(Info.StripIdx, 0);
  ASSERT_EQ(Info.SliceColIdx, 0);
  ASSERT_EQ(Info.SliceRowIdx, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer2Pixel1) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 0;
  Wanted.StripIdx = 1;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 0);
  ASSERT_EQ(Info.StripIdx, 1);
  ASSERT_EQ(Info.SliceColIdx, 0);
  ASSERT_EQ(Info.SliceRowIdx, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer3Sector2_TopLeft) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.SliceColIdx = 0;
  Wanted.SliceRowIdx = 0;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 1);
  ASSERT_EQ(Info.StripIdx, 2);
  ASSERT_EQ(Info.SliceColIdx, 0);
  ASSERT_EQ(Info.SliceRowIdx, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer3Sector2_TopRight) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.SliceColIdx = 15;
  Wanted.SliceRowIdx = 0;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 1);
  ASSERT_EQ(Info.StripIdx, 2);
  ASSERT_EQ(Info.SliceColIdx, 15);
  ASSERT_EQ(Info.SliceRowIdx, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer3Sector2_BottomRight) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.SliceColIdx = 15;
  Wanted.SliceRowIdx = 15;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 1);
  ASSERT_EQ(Info.StripIdx, 2);
  ASSERT_EQ(Info.SliceColIdx, 15);
  ASSERT_EQ(Info.SliceRowIdx, 15);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer3Sector2_BottomLeft) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.SliceColIdx = 0;
  Wanted.SliceRowIdx = 15;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 1);
  ASSERT_EQ(Info.StripIdx, 2);
  ASSERT_EQ(Info.SliceColIdx, 0);
  ASSERT_EQ(Info.SliceRowIdx, 15);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_BottomMost) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 22;
  Wanted.StripIdx = 15;
  Wanted.SliceColIdx = 15;
  Wanted.SliceRowIdx = 15;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 22);
  ASSERT_EQ(Info.StripIdx, 15);
  ASSERT_EQ(Info.SliceColIdx, 15);
  ASSERT_EQ(Info.SliceRowIdx, 15);
}

TEST_F(DreamIcdTest, SumoStartColOffsetFromSliceCol_) {
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(0+0), 0);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(0+1), 0);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(0+18), 0);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(0+19), 0);
  
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(20+0), 20);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(20+1), 20);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(20+14), 20);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(20+15), 20);
  
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(36+0), 36);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(36+1), 36);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(36+10), 36);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(36+11), 36);

  ASSERT_EQ(SumoStartColOffsetFromSliceCol(48+0), 48);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(48+1), 48);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(48+6), 48);
  ASSERT_EQ(SumoStartColOffsetFromSliceCol(48+7), 48);
}

class JalConfigTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
  std::string TestJsonPath{TEST_DATA_PATH};
};

/** Test cases below */

// \todo do proper tests

TEST_F(JalConfigTest, ConstructorDefaults) {
  Jalousie::Config config;
  MESSAGE() << config.debug();
  //  ASSERT_FALSE(config.spoof_high_time);
  //  ASSERT_EQ(config.reduction_strategy, "");
  //  ASSERT_EQ(config.mappings.max_z(), 0);
  //  ASSERT_FALSE(config.mappings.is_valid(0, 0, 1000));
  //  ASSERT_FALSE(config.mappings.is_valid(1, 0, 1000));
  //  ASSERT_FALSE(config.mappings.is_valid(2, 0, 1000));
}

TEST_F(JalConfigTest, ValidConfig) {
  Jalousie::Config config(TestJsonPath + "v20_mappings.json");
  MESSAGE() << config.debug();
  //  ASSERT_TRUE(config.spoof_high_time);
  //  ASSERT_EQ(config.reduction_strategy, "maximum");
  //  ASSERT_EQ(config.mappings.max_z(), 20);
  //  ASSERT_TRUE(config.mappings.is_valid(0, 0, 1000));
  //  ASSERT_TRUE(config.mappings.is_valid(1, 0, 1000));
  //  ASSERT_TRUE(config.mappings.is_valid(2, 0, 1000));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
