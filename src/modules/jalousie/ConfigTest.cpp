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
  uint8_t ColIdx;
  uint8_t Width;
  uint8_t Sumo;
};

// The layout of the Sumo types along a SliceRow, indexd by SliceColIdx
// 66666666666666666666555555555555555544444444444433333333
SumoColWidth SumoColWidthFromSliceCol(int SliceColIdx) {
  // clang-format off
  static const SumoColWidth SliceSumoStartColOffset_Width_Sumo[14] = {
    {  0, 20, 6 }, {  0, 20, 6 }, {  0, 20, 6 }, {  0, 20, 6 }, { 0, 20, 6}, //  0- 4, SliceColIdx  0-19: Sumo 6, Cols 1-20
    { 20, 16, 5 }, { 20, 16, 5 }, { 20, 16, 5 }, { 20, 16, 5 },              //  5- 8, SliceColIdx 20-35: Sumo 5, Cols 1-16
    { 36, 12, 4 }, { 36, 12, 4 }, { 36, 12, 4 },                             //  9-11, SliceColIdx 36-47: Sumo 4, Cols 1-12
    { 48,  8, 3 }, { 48,  8, 3 },                                            // 12-13, SliceColIdx 48-55: Sumo 3, Cols 1-8
  };
  // clang-format on
  int SliceColDiv4 = SliceColIdx / 4;
  SumoColWidth SumoColWidth = SliceSumoStartColOffset_Width_Sumo[SliceColDiv4];
  SumoColWidth.ColIdx = uint8_t(
      SliceColIdx - SumoColWidth.ColIdx); // map StartColOffset to SumoCol
  return SumoColWidth;
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

struct PhysicalCoords {
  int Sector;
  int Sumo;
  int Strip;
  int Wire;
  int Cassette;
  int Counter;
};

PhysicalCoords PhysicalCoordsFromPixelId(int PixelId) {
  SliceInfo SliceCoords = PixelIdToSliceInfo(PixelId);
  /// \todo make this SumoCoords or the like
  SumoColWidth SumoCoords = SumoColWidthFromSliceCol(SliceCoords.SliceColIdx);
  StripPlaneCoord StripCoords = StripPlaneCoordFromSumoLocalCoord(
      SumoCoords.ColIdx, SliceCoords.SliceRowIdx, SumoCoords.Width);

  PhysicalCoords Phys;
  Phys.Sector = SliceCoords.SectorIdx + 1;
  Phys.Sumo = SumoCoords.Sumo;
  Phys.Strip = SliceCoords.StripIdx + 1;
  Phys.Wire = StripCoords.WireIdx + 1;
  Phys.Cassette = StripCoords.CassetteIdx + 1;
  Phys.Counter = StripCoords.CounterIdx + 1;
  
  return Phys;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

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
  Wanted.SliceColIdx = SliceWidth - 1;
  Wanted.SliceRowIdx = 0;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 1);
  ASSERT_EQ(Info.StripIdx, 2);
  ASSERT_EQ(Info.SliceColIdx, SliceWidth - 1);
  ASSERT_EQ(Info.SliceRowIdx, 0);
}

TEST_F(DreamIcdTest, PixelIdToSliceInfo_StripLayer3Sector2_BottomRight) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.SliceColIdx = SliceWidth - 1;
  Wanted.SliceRowIdx = 15;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  SliceInfo Info = PixelIdToSliceInfo(PixelId);
  ASSERT_EQ(Info.SectorIdx, 1);
  ASSERT_EQ(Info.StripIdx, 2);
  ASSERT_EQ(Info.SliceColIdx, SliceWidth - 1);
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

TEST_F(DreamIcdTest, SumoColWidthFromSliceCol_TwoFirstAndLast) {
  // Sumo 6
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 00).ColIdx, 00);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 01).ColIdx, 01);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 18).ColIdx, 18);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 19).ColIdx, 19);

  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 00).Width, 20);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 01).Width, 20);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 18).Width, 20);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 19).Width, 20);

  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 00).ColIdx, 00);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 01).ColIdx, 01);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 18).ColIdx, 18);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 19).ColIdx, 19);

  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 00).Sumo, 6);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 01).Sumo, 6);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 18).Sumo, 6);
  ASSERT_EQ(SumoColWidthFromSliceCol(0 + 19).Sumo, 6);

  // Sumo 5
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 00).ColIdx, 00);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 01).ColIdx, 01);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 14).ColIdx, 14);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 15).ColIdx, 15);

  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 00).Width, 16);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 01).Width, 16);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 14).Width, 16);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 15).Width, 16);

  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 00).Sumo, 5);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 01).Sumo, 5);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 14).Sumo, 5);
  ASSERT_EQ(SumoColWidthFromSliceCol(20 + 15).Sumo, 5);

  // Sumo 4
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 00).ColIdx, 00);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 01).ColIdx, 01);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 10).ColIdx, 10);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 11).ColIdx, 11);

  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 00).Width, 12);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 01).Width, 12);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 10).Width, 12);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 11).Width, 12);

  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 00).Sumo, 4);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 01).Sumo, 4);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 10).Sumo, 4);
  ASSERT_EQ(SumoColWidthFromSliceCol(36 + 11).Sumo, 4);

  // Sumo 3
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 0).ColIdx, 0);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 1).ColIdx, 1);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 6).ColIdx, 6);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 7).ColIdx, 7);

  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 0).Width, 8);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 1).Width, 8);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 6).Width, 8);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 7).Width, 8);

  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 0).Sumo, 3);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 1).Sumo, 3);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 6).Sumo, 3);
  ASSERT_EQ(SumoColWidthFromSliceCol(48 + 7).Sumo, 3);
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

TEST_F(DreamIcdTest, PhysicalCoordsFromPixelId_Pixel1) {
  int PixelId = 1;
  PhysicalCoords Phys = PhysicalCoordsFromPixelId(PixelId);
  ASSERT_EQ(Phys.Sector, 1);
  ASSERT_EQ(Phys.Sumo, 6);
  ASSERT_EQ(Phys.Strip, 1);
  ASSERT_EQ(Phys.Wire, 16);
  ASSERT_EQ(Phys.Cassette, 10);
  ASSERT_EQ(Phys.Counter, 2); 
}

TEST_F(DreamIcdTest, PhysicalCoordsFromPixelId_Sector3_BottomLeft) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 2;
  Wanted.SliceRowIdx = 15;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  PhysicalCoords Phys = PhysicalCoordsFromPixelId(PixelId);
  ASSERT_EQ(Phys.Sector, 3);
  ASSERT_EQ(Phys.Sumo, 6);
  ASSERT_EQ(Phys.Strip, 1);
  ASSERT_EQ(Phys.Wire, 1);
  ASSERT_EQ(Phys.Cassette, 10);
  ASSERT_EQ(Phys.Counter, 2); 
}


TEST_F(DreamIcdTest, PhysicalCoordsFromPixelId_StripLayer3Sector2_TopLeft) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.SliceColIdx = 0;
  Wanted.SliceRowIdx = 0;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  PhysicalCoords Phys = PhysicalCoordsFromPixelId(PixelId);
  ASSERT_EQ(Phys.Sector, 2);
  ASSERT_EQ(Phys.Sumo, 6);
  ASSERT_EQ(Phys.Strip, 3);
  ASSERT_EQ(Phys.Wire, 16);
  ASSERT_EQ(Phys.Cassette, 10);
  ASSERT_EQ(Phys.Counter, 2); 
}

TEST_F(DreamIcdTest, PhysicalCoordsFromPixelId_StripLayer3Sector2_TopRight) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.SliceColIdx = SliceWidth - 1;
  Wanted.SliceRowIdx = 0;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  PhysicalCoords Phys = PhysicalCoordsFromPixelId(PixelId);
  ASSERT_EQ(Phys.Sector, 2);
  ASSERT_EQ(Phys.Sumo, 3);
  ASSERT_EQ(Phys.Strip, 3);
  ASSERT_EQ(Phys.Wire, 16);
  ASSERT_EQ(Phys.Cassette, 1);
  ASSERT_EQ(Phys.Counter, 1); 
}

TEST_F(DreamIcdTest, PhysicalCoordsFromPixelId_StripLayer3Sector2_BottomRight) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.SliceColIdx = SliceWidth - 1;
  Wanted.SliceRowIdx = 15;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  PhysicalCoords Phys = PhysicalCoordsFromPixelId(PixelId);
  ASSERT_EQ(Phys.Sector, 2);
  ASSERT_EQ(Phys.Sumo, 3);
  ASSERT_EQ(Phys.Strip, 3);
  ASSERT_EQ(Phys.Wire, 1);
  ASSERT_EQ(Phys.Cassette, 1);
  ASSERT_EQ(Phys.Counter, 1); 
}

TEST_F(DreamIcdTest, PhysicalCoordsFromPixelId_StripLayer3Sector2_BottomLeft) {
  SliceInfo Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.SliceColIdx = 0;
  Wanted.SliceRowIdx = 15;
  int PixelId = PixelIdFromSliceInfo(Wanted);
  PhysicalCoords Phys = PhysicalCoordsFromPixelId(PixelId);
  ASSERT_EQ(Phys.Sector, 2);
  ASSERT_EQ(Phys.Sumo, 6);
  ASSERT_EQ(Phys.Strip, 3);
  ASSERT_EQ(Phys.Wire, 1);
  ASSERT_EQ(Phys.Cassette, 10);
  ASSERT_EQ(Phys.Counter, 2); 
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


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
