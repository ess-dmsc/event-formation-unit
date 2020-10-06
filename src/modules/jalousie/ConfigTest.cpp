/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <jalousie/Config.h>
#include <test/TestBase.h>

class DreamIcdTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/// \todo Handle pixel min/max value test?
/// \todo Benchmark signed/unsigned int. Unsigned looks much shorter on godbolt.

// test global pixel to sumo slice mapping

/// A Slice refers to a Sector Sumo Slice, which is a slice of a Sector along
/// the Strip axis. \todo rename to PixelInSlice? or SlicePixel
struct SliceInfo {
  int SectorIdx;
  int StripIdx;
  int SliceColIdx;
  int SliceRowIdx;
};

enum SliceMapConstants {
  SliceWidth =
      56, /// Width of the Sector Sumo Slice in logical coordinates, in pixels.
  SliceHeight =
      16, /// Width of the Sector Sumo Slice in logical coordinates, in pixels.
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

// todo refactor to model row as well?, to get pixel-in-sumo representation
struct SumoColWidth {
  uint8_t SumoColIdx;
  uint8_t Width;
};

// The layout of the Sumo types along a SliceRow, indexd by SliceColIdx
// 66666666666666666666555555555555555544444444444433333333
SumoColWidth SumoColWidthFromSliceCol(int SliceColIdx) {
  // clang-format off
  static const uint8_t SliceSumoStartColOffsetAndWidth[14][2] = {
    { 0, 20}, { 0, 20}, { 0, 20}, { 0, 20}, { 0, 20}, //  0- 4, SliceColIdx  0-19: Sumo 6, Cols 1-20
    {20, 16}, {20, 16}, {20, 16}, {20, 16},           //  5- 8, SliceColIdx 20-35: Sumo 5, Cols 1-16
    {36, 12}, {36, 12}, {36, 12},                     //  9-11, SliceColIdx 36-47: Sumo 4, Cols 1-12
    {48,  8}, {48,  8},                               // 12-13, SliceColIdx 48-55: Sumo 3, Cols 1-8
  };
  // clang-format on
  int SliceColDiv4 = SliceColIdx / 4;
  SumoColWidth ColWidth{
      uint8_t(SliceColIdx - SliceSumoStartColOffsetAndWidth[SliceColDiv4][0]),
      SliceSumoStartColOffsetAndWidth[SliceColDiv4][1]};
  return ColWidth;
}

struct StripPlaneCoord {
  int WireIdx;
  int CassetteIdx;
  int CounterIdx;
};

StripPlaneCoord StripPlaneCoordFromSumoLocalCoord(int SumoColIdx,
                                                  int SumoRowIdx,
                                                  int SumoWidth) {
  StripPlaneCoord Coord;
  int CassetteCounterIdx = SumoWidth - SumoColIdx - 1;
  Coord.CassetteIdx = CassetteCounterIdx / 2;
  Coord.CounterIdx = CassetteCounterIdx % 2;
  Coord.WireIdx = SliceHeight - SumoRowIdx - 1;
  return Coord;
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

TEST_F(DreamIcdTest, SumoStartColWidthFromSliceCol_TwoFirstAndLast) {
  // Sumo 6
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 00).SumoColIdx, 00);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 01).SumoColIdx, 01);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 18).SumoColIdx, 18);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 19).SumoColIdx, 19);

  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 00).Width, 20);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 01).Width, 20);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 18).Width, 20);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 19).Width, 20);

  // Sumo 5
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 00).SumoColIdx, 00);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 01).SumoColIdx, 01);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 14).SumoColIdx, 14);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 15).SumoColIdx, 15);

  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 00).Width, 16);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 01).Width, 16);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 14).Width, 16);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 15).Width, 16);

  // Sumo 4
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 00).SumoColIdx, 00);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 01).SumoColIdx, 01);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 10).SumoColIdx, 10);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 11).SumoColIdx, 11);

  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 00).Width, 12);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 01).Width, 12);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 10).Width, 12);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 11).Width, 12);

  // Sumo 3
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 0).SumoColIdx, 0);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 1).SumoColIdx, 1);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 6).SumoColIdx, 6);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 7).SumoColIdx, 7);

  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 0).Width, 8);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 1).Width, 8);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 6).Width, 8);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 7).Width, 8);
}

//
// Sumo 6
//
TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo6_TopLeft) {
  int SumoColIdx = 0;
  int SumoRowIdx = 0;
  int SumoWidth = 20; // Sumo 6
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 9);
  ASSERT_EQ(Coord.CounterIdx, 1);
  ASSERT_EQ(Coord.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo6_TopRight) {
  int SumoColIdx = 19;
  int SumoRowIdx = 0;
  int SumoWidth = 20; // Sumo 6
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 0);
  ASSERT_EQ(Coord.CounterIdx, 0);
  ASSERT_EQ(Coord.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo6_BottomRight) {
  int SumoColIdx = 19;
  int SumoRowIdx = 15;
  int SumoWidth = 20; // Sumo 6
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 0);
  ASSERT_EQ(Coord.CounterIdx, 0);
  ASSERT_EQ(Coord.WireIdx, 0);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo6_BottomLeft) {
  int SumoColIdx = 0;
  int SumoRowIdx = 15;
  int SumoWidth = 20; // Sumo 6
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 9);
  ASSERT_EQ(Coord.CounterIdx, 1);
  ASSERT_EQ(Coord.WireIdx, 0);
}

//
// Sumo 5
//
TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo5_TopLeft) {
  int SumoColIdx = 0;
  int SumoRowIdx = 0;
  int SumoWidth = 16; // Sumo 5
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 7);
  ASSERT_EQ(Coord.CounterIdx, 1);
  ASSERT_EQ(Coord.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo5_TopRight) {
  int SumoColIdx = 15;
  int SumoRowIdx = 0;
  int SumoWidth = 16; // Sumo 5
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 0);
  ASSERT_EQ(Coord.CounterIdx, 0);
  ASSERT_EQ(Coord.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo5_BottomRight) {
  int SumoColIdx = 15;
  int SumoRowIdx = 15;
  int SumoWidth = 16; // Sumo 5
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 0);
  ASSERT_EQ(Coord.CounterIdx, 0);
  ASSERT_EQ(Coord.WireIdx, 0);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo5_BottomLeft) {
  int SumoColIdx = 0;
  int SumoRowIdx = 15;
  int SumoWidth = 16; // Sumo 5
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 7);
  ASSERT_EQ(Coord.CounterIdx, 1);
  ASSERT_EQ(Coord.WireIdx, 0);
}

//
// Sumo 4
//
TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo4_TopLeft) {
  int SumoColIdx = 0;
  int SumoRowIdx = 0;
  int SumoWidth = 12; // Sumo 4
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 5);
  ASSERT_EQ(Coord.CounterIdx, 1);
  ASSERT_EQ(Coord.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo4_TopRight) {
  int SumoColIdx = 11;
  int SumoRowIdx = 0;
  int SumoWidth = 12; // Sumo 4
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 0);
  ASSERT_EQ(Coord.CounterIdx, 0);
  ASSERT_EQ(Coord.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo4_BottomRight) {
  int SumoColIdx = 11;
  int SumoRowIdx = 15;
  int SumoWidth = 12; // Sumo 4
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 0);
  ASSERT_EQ(Coord.CounterIdx, 0);
  ASSERT_EQ(Coord.WireIdx, 0);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo4_BottomLeft) {
  int SumoColIdx = 0;
  int SumoRowIdx = 15;
  int SumoWidth = 12; // Sumo 4
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 5);
  ASSERT_EQ(Coord.CounterIdx, 1);
  ASSERT_EQ(Coord.WireIdx, 0);
}

//
// Sumo 3
//
TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo3_TopLeft) {
  int SumoColIdx = 0;
  int SumoRowIdx = 0;
  int SumoWidth = 8; // Sumo 3
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 3);
  ASSERT_EQ(Coord.CounterIdx, 1);
  ASSERT_EQ(Coord.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo3_TopRight) {
  int SumoColIdx = 7;
  int SumoRowIdx = 0;
  int SumoWidth = 8; // Sumo 3
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 0);
  ASSERT_EQ(Coord.CounterIdx, 0);
  ASSERT_EQ(Coord.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo3_BottomRight) {
  int SumoColIdx = 7;
  int SumoRowIdx = 15;
  int SumoWidth = 8; // Sumo 3
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 0);
  ASSERT_EQ(Coord.CounterIdx, 0);
  ASSERT_EQ(Coord.WireIdx, 0);
}

TEST_F(DreamIcdTest, StripPlaneCoordFromSumoLocalCoord_Sumo3_BottomLeft) {
  int SumoColIdx = 0;
  int SumoRowIdx = 15;
  int SumoWidth = 8; // Sumo 3
  StripPlaneCoord Coord =
      StripPlaneCoordFromSumoLocalCoord(SumoColIdx, SumoRowIdx, SumoWidth);
  ASSERT_EQ(Coord.CassetteIdx, 3);
  ASSERT_EQ(Coord.CounterIdx, 1);
  ASSERT_EQ(Coord.WireIdx, 0);
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
