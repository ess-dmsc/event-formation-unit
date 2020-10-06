/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/Config.h>
#include <test/TestBase.h>

class DreamIcdTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/// \todo Handle pixel min/max value test?

// test global pixel to sumo slice mapping

struct SliceInfo {
  int SectorZ;
  int StripZ;
  int SliceColZ;
  int SliceRowZ;
};

enum SliceMapConstants {
  SliceHeight = 16,
  SliceWidth = 56,
  SectorCount = 23,
  TotalWidth = SliceWidth * SectorCount
};

/// \brief this maps pixelid to the SectorStripSlice. SliceRowZ and SliceColZ
/// are coordinates inside the slice.
SliceInfo PixelIdToSliceInfo(int PixelId) {
  SliceInfo Info;
  int PixelIdZ = PixelId - 1;
  Info.SectorZ = (PixelIdZ / SliceWidth) % SectorCount;
  Info.StripZ = (PixelIdZ / TotalWidth) / SliceHeight;
  Info.SliceColZ = PixelIdZ % SliceWidth;
  Info.SliceRowZ = (PixelIdZ / TotalWidth) % SliceHeight;
  return Info;
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_Pixel1) {
  int PixelId = 1;
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorZ, 0);
  ASSERT_EQ(Info.StripZ, 0);
  ASSERT_EQ(Info.SliceColZ, 0);
  ASSERT_EQ(Info.SliceRowZ, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_Sector2Pixel1) {
  int PixelId = 1 + SliceWidth;
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorZ, 1);
  ASSERT_EQ(Info.StripZ, 0);
  ASSERT_EQ(Info.SliceColZ, 0);
  ASSERT_EQ(Info.SliceRowZ, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_Sector3Pixel1) {
  int PixelId = 1 + 2 * SliceWidth;
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorZ, 2);
  ASSERT_EQ(Info.StripZ, 0);
  ASSERT_EQ(Info.SliceColZ, 0);
  ASSERT_EQ(Info.SliceRowZ, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer2Pixel1) {
  int WantedSectorZ = 0;
  int WantedStripZ = 1;
  int PixelId = 1 + WantedSectorZ * SliceWidth +
                WantedStripZ * (SliceWidth * SectorCount * SliceHeight);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorZ, 0);
  ASSERT_EQ(Info.StripZ, 1);
  ASSERT_EQ(Info.SliceColZ, 0);
  ASSERT_EQ(Info.SliceRowZ, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer3Sector2_TopLeft) {
  int WantedSectorZ = 1;
  int WantedStripZ = 2;
  int WantedSliceColZ = 0;
  int WantedSliceRowZ = 0;
  int PixelId = 1 + WantedSliceColZ +
                WantedSectorZ * (SliceWidth) +
                WantedSliceRowZ * (SliceWidth * SectorCount) +
                WantedStripZ * (SliceWidth * SectorCount * SliceHeight);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorZ, 1);
  ASSERT_EQ(Info.StripZ, 2);
  ASSERT_EQ(Info.SliceColZ, 0);
  ASSERT_EQ(Info.SliceRowZ, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer3Sector2_TopRight) {
  int WantedSectorZ = 1;
  int WantedStripZ = 2;
  int WantedSliceColZ = 15;
  int WantedSliceRowZ = 0;
  int PixelId = 1 + WantedSliceColZ +
                WantedSectorZ * (SliceWidth) +
                WantedSliceRowZ * (SliceWidth * SectorCount) +
                WantedStripZ * (SliceWidth * SectorCount * SliceHeight);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorZ, 1);
  ASSERT_EQ(Info.StripZ, 2);
  ASSERT_EQ(Info.SliceColZ, 15);
  ASSERT_EQ(Info.SliceRowZ, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer3Sector2_BottomRight) {
  int WantedSectorZ = 1;
  int WantedStripZ = 2;
  int WantedSliceColZ = 15;
  int WantedSliceRowZ = 15;
  int PixelId = 1 + WantedSliceColZ +
                WantedSectorZ * (SliceWidth) +
                WantedSliceRowZ * (SliceWidth * SectorCount) +
                WantedStripZ * (SliceWidth * SectorCount * SliceHeight);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorZ, 1);
  ASSERT_EQ(Info.StripZ, 2);
  ASSERT_EQ(Info.SliceColZ, 15);
  ASSERT_EQ(Info.SliceRowZ, 15);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer3Sector2_BottomLeft) {
  int WantedSectorZ = 1;
  int WantedStripZ = 2;
  int WantedSliceColZ = 0;
  int WantedSliceRowZ = 15;
  int PixelId = 1 + WantedSliceColZ +
                WantedSectorZ * (SliceWidth) +
                WantedSliceRowZ * (SliceWidth * SectorCount) +
                WantedStripZ * (SliceWidth * SectorCount * SliceHeight);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorZ, 1);
  ASSERT_EQ(Info.StripZ, 2);
  ASSERT_EQ(Info.SliceColZ, 0);
  ASSERT_EQ(Info.SliceRowZ, 15);
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
