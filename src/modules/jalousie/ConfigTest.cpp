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
/// the Strip axis.
struct SlicePixel {
  int SectorIdx;
  int StripIdx;
  int X;
  int Y;
};

struct SumoPixel {
  uint8_t X;
  uint8_t Y;
  uint8_t Width;
  uint8_t Sumo;
};

struct StripPlanePixel {
  int WireIdx;
  int CassetteIdx;
  int CounterIdx;
};

struct EndCapParams {
  int Sector;
  int Sumo;
  int Strip;
  int Wire;
  int Cassette;
  int Counter;
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
/// \todo can this be changed to "masking" by doing PixelIdFromSlicePixel() in
/// "reverse"?
SlicePixel SlicePixelFromPixelId(int PixelId) {
  SlicePixel Slice;
  int PixelIdx = PixelId - 1;
  int SectorIdx = PixelIdx / SliceWidth;
  int GlobalY = PixelIdx / TotalWidth;
  Slice.SectorIdx = SectorIdx % SectorCount;
  Slice.StripIdx = GlobalY / SliceHeight;
  Slice.X = PixelIdx % SliceWidth;
  Slice.Y = GlobalY % SliceHeight;
  return Slice;
}

int PixelIdFromSlicePixel(SlicePixel Slice) {
  int PixelId = 1 + Slice.X + Slice.SectorIdx * (SliceWidth) +
                Slice.Y * (SliceWidth * SectorCount) +
                Slice.StripIdx * (SliceWidth * SectorCount * SliceHeight);
  return PixelId;
}

// The layout of the Sumo types along a SliceRow, indexd by X
// 66666666666666666666555555555555555544444444444433333333
SumoPixel SumoPixelFromSlicePixel(SlicePixel Slice) {
  struct SliceToSumoProperty {
    uint8_t SumoStartOffsetX;
    uint8_t SumoWidth;
    uint8_t SumoId;
  };
  // clang-format off
  static const SliceToSumoProperty SliceToSumoMap[14] = {
    {  0, 20, 6 }, {  0, 20, 6 }, {  0, 20, 6 }, {  0, 20, 6 }, { 0, 20, 6}, //  0- 4, SlicePixel.X  0-19: Sumo 6, Cols 1-20
    { 20, 16, 5 }, { 20, 16, 5 }, { 20, 16, 5 }, { 20, 16, 5 },              //  5- 8, SlicePixel.X 20-35: Sumo 5, Cols 1-16
    { 36, 12, 4 }, { 36, 12, 4 }, { 36, 12, 4 },                             //  9-11, SlicePixel.X 36-47: Sumo 4, Cols 1-12
    { 48,  8, 3 }, { 48,  8, 3 },                                            // 12-13, SlicePixel.X 48-55: Sumo 3, Cols 1-8
  };
  // clang-format on
  int XCompact = Slice.X / 4; // Range reduced from 56 to 14 -> fewer constants
  SliceToSumoProperty SliceToSumo = SliceToSumoMap[XCompact];

  SumoPixel Sumo;
  Sumo.X = uint8_t(Slice.X - SliceToSumo.SumoStartOffsetX);
  Sumo.Y = Slice.Y;
  Sumo.Width = SliceToSumo.SumoWidth;
  Sumo.Sumo = SliceToSumo.SumoId;
  return Sumo;
}

StripPlanePixel StripPlanePixelFromSumoPixel(SumoPixel Sumo) {
  StripPlanePixel StripPlane;
  int CassetteCounterIdx = Sumo.Width - Sumo.X - 1;
  StripPlane.CassetteIdx = CassetteCounterIdx / 2;
  StripPlane.CounterIdx = CassetteCounterIdx % 2;
  StripPlane.WireIdx = SliceHeight - Sumo.Y - 1;
  return StripPlane;
}

EndCapParams EndCapCoordsFromPixelId(int PixelId) {
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  SumoPixel Sumo = SumoPixelFromSlicePixel(Slice);
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);

  EndCapParams EndCap;
  EndCap.Sector = Slice.SectorIdx + 1;
  EndCap.Sumo = Sumo.Sumo;
  EndCap.Strip = Slice.StripIdx + 1;
  EndCap.Wire = StripPlane.WireIdx + 1;
  EndCap.Cassette = StripPlane.CassetteIdx + 1;
  EndCap.Counter = StripPlane.CounterIdx + 1;

  return EndCap;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TEST_F(DreamIcdTest, SlicePixelFromPixelId_Pixel1) {
  int PixelId = 1;
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamIcdTest, SlicePixelFromPixelId_Sector2Pixel1) {
  int PixelId = 1 + SliceWidth;
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 1);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamIcdTest, SlicePixelFromPixelId_Sector3Pixel1) {
  int PixelId = 1 + 2 * SliceWidth;
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 2);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamIcdTest, SlicePixelFromPixelId_StripLayer2Pixel1) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 0;
  Wanted.StripIdx = 1;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 1);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamIcdTest, SlicePixelFromPixelId_StripLayer3Sector2_TopLeft) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = 0;
  Wanted.Y = 0;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 1);
  ASSERT_EQ(Slice.StripIdx, 2);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamIcdTest, SlicePixelFromPixelId_StripLayer3Sector2_TopRight) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = SliceWidth - 1;
  Wanted.Y = 0;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 1);
  ASSERT_EQ(Slice.StripIdx, 2);
  ASSERT_EQ(Slice.X, SliceWidth - 1);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamIcdTest, SlicePixelFromPixelId_StripLayer3Sector2_BottomRight) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = SliceWidth - 1;
  Wanted.Y = 15;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 1);
  ASSERT_EQ(Slice.StripIdx, 2);
  ASSERT_EQ(Slice.X, SliceWidth - 1);
  ASSERT_EQ(Slice.Y, 15);
}

TEST_F(DreamIcdTest, SlicePixelFromPixelId_StripLayer3Sector2_BottomLeft) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = 0;
  Wanted.Y = 15;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 1);
  ASSERT_EQ(Slice.StripIdx, 2);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 15);
}

TEST_F(DreamIcdTest, SlicePixelFromPixelId_BottomMost) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 22;
  Wanted.StripIdx = 15;
  Wanted.X = 15;
  Wanted.Y = 15;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 22);
  ASSERT_EQ(Slice.StripIdx, 15);
  ASSERT_EQ(Slice.X, 15);
  ASSERT_EQ(Slice.Y, 15);
}

TEST_F(DreamIcdTest, SumoPixelFromSlicePixel_TwoFirstAndLast) {
  auto MakeSlicePixel = [](int X) { return SlicePixel{0, 0, X, 0}; };

  // Sumo 6
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 00)).X, 00);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 01)).X, 01);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 18)).X, 18);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 19)).X, 19);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 00)).Width, 20);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 01)).Width, 20);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 18)).Width, 20);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 19)).Width, 20);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 00)).X, 00);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 01)).X, 01);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 18)).X, 18);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 19)).X, 19);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 00)).Sumo, 6);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 01)).Sumo, 6);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 18)).Sumo, 6);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 19)).Sumo, 6);

  // Sumo 5
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 00)).X, 00);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 01)).X, 01);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 14)).X, 14);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 15)).X, 15);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 00)).Width, 16);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 01)).Width, 16);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 14)).Width, 16);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 15)).Width, 16);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 00)).Sumo, 5);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 01)).Sumo, 5);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 14)).Sumo, 5);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 15)).Sumo, 5);

  // Sumo 4
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 00)).X, 00);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 01)).X, 01);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 10)).X, 10);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 11)).X, 11);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 00)).Width, 12);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 01)).Width, 12);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 10)).Width, 12);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 11)).Width, 12);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 00)).Sumo, 4);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 01)).Sumo, 4);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 10)).Sumo, 4);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 11)).Sumo, 4);

  // Sumo 3
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 0)).X, 0);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 1)).X, 1);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 6)).X, 6);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 7)).X, 7);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 0)).Width, 8);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 1)).Width, 8);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 6)).Width, 8);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 7)).Width, 8);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 0)).Sumo, 3);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 1)).Sumo, 3);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 6)).Sumo, 3);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 7)).Sumo, 3);
}

//
// Sumo 6
//
TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo6_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 20; // Sumo 6
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 9);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo6_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 19;
  Sumo.Y = 0;
  Sumo.Width = 20; // Sumo 6
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo6_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 19;
  Sumo.Y = 15;
  Sumo.Width = 20; // Sumo 6
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo6_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 20; // Sumo 6
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 9);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

//
// Sumo 5
//
TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo5_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 16; // Sumo 5
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 7);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo5_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 15;
  Sumo.Y = 0;
  Sumo.Width = 16; // Sumo 5
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo5_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 15;
  Sumo.Y = 15;
  Sumo.Width = 16; // Sumo 5
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo5_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 16; // Sumo 5
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 7);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

//
// Sumo 4
//
TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo4_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 12; // Sumo 4
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 5);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo4_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 11;
  Sumo.Y = 0;
  Sumo.Width = 12; // Sumo 4
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo4_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 11;
  Sumo.Y = 15;
  Sumo.Width = 12; // Sumo 4
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo4_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 12; // Sumo 4
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 5);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

//
// Sumo 3
//
TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo3_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 8; // Sumo 3
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 3);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo3_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 7;
  Sumo.Y = 0;
  Sumo.Width = 8; // Sumo 3
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo3_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 7;
  Sumo.Y = 15;
  Sumo.Width = 8; // Sumo 3
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

TEST_F(DreamIcdTest, StripPlanePixelFromSumoPixel_Sumo3_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 8; // Sumo 3
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 3);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

TEST_F(DreamIcdTest, EndCapCoordsFromPixelId_Pixel1) {
  int PixelId = 1;
  EndCapParams EndCap = EndCapCoordsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 1);
  ASSERT_EQ(EndCap.Sumo, 6);
  ASSERT_EQ(EndCap.Strip, 1);
  ASSERT_EQ(EndCap.Wire, 16);
  ASSERT_EQ(EndCap.Cassette, 10);
  ASSERT_EQ(EndCap.Counter, 2);
}

TEST_F(DreamIcdTest, EndCapCoordsFromPixelId_Sector3_BottomLeft) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 2;
  Wanted.Y = 15;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapCoordsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 3);
  ASSERT_EQ(EndCap.Sumo, 6);
  ASSERT_EQ(EndCap.Strip, 1);
  ASSERT_EQ(EndCap.Wire, 1);
  ASSERT_EQ(EndCap.Cassette, 10);
  ASSERT_EQ(EndCap.Counter, 2);
}

TEST_F(DreamIcdTest, EndCapCoordsFromPixelId_StripLayer3Sector2_TopLeft) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = 0;
  Wanted.Y = 0;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapCoordsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.Sumo, 6);
  ASSERT_EQ(EndCap.Strip, 3);
  ASSERT_EQ(EndCap.Wire, 16);
  ASSERT_EQ(EndCap.Cassette, 10);
  ASSERT_EQ(EndCap.Counter, 2);
}

TEST_F(DreamIcdTest, EndCapCoordsFromPixelId_StripLayer3Sector2_TopRight) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = SliceWidth - 1;
  Wanted.Y = 0;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapCoordsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.Sumo, 3);
  ASSERT_EQ(EndCap.Strip, 3);
  ASSERT_EQ(EndCap.Wire, 16);
  ASSERT_EQ(EndCap.Cassette, 1);
  ASSERT_EQ(EndCap.Counter, 1);
}

TEST_F(DreamIcdTest, EndCapCoordsFromPixelId_StripLayer3Sector2_BottomRight) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = SliceWidth - 1;
  Wanted.Y = 15;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapCoordsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.Sumo, 3);
  ASSERT_EQ(EndCap.Strip, 3);
  ASSERT_EQ(EndCap.Wire, 1);
  ASSERT_EQ(EndCap.Cassette, 1);
  ASSERT_EQ(EndCap.Counter, 1);
}

TEST_F(DreamIcdTest, EndCapCoordsFromPixelId_StripLayer3Sector2_BottomLeft) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = 0;
  Wanted.Y = 15;
  int PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapCoordsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.Sumo, 6);
  ASSERT_EQ(EndCap.Strip, 3);
  ASSERT_EQ(EndCap.Wire, 1);
  ASSERT_EQ(EndCap.Cassette, 10);
  ASSERT_EQ(EndCap.Counter, 2);
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
