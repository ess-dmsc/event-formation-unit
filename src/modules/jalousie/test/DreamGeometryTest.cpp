#include <test/TestBase.h>

#include <jalousie/geometry/DreamGeometry.h>

class DreamGeometryTest : public TestBase {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DreamGeometryTest, SlicePixelFromPixelId_Pixel1) {
  uint32_t PixelId = 1;
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamGeometryTest, SlicePixelFromPixelId_Sector2Pixel1) {
  uint32_t PixelId = 1 + DreamGeometry::SliceWidth;
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 1);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamGeometryTest, SlicePixelFromPixelId_Sector3Pixel1) {
  uint32_t PixelId = 1 + 2 * DreamGeometry::SliceWidth;
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 2);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamGeometryTest, SlicePixelFromPixelId_StripLayer2Pixel1) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 0;
  Wanted.StripIdx = 1;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 1);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamGeometryTest, SlicePixelFromPixelId_StripLayer3Sector2_TopLeft) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = 0;
  Wanted.Y = 0;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 1);
  ASSERT_EQ(Slice.StripIdx, 2);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamGeometryTest, SlicePixelFromPixelId_StripLayer3Sector2_TopRight) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = DreamGeometry::SliceWidth - 1;
  Wanted.Y = 0;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 1);
  ASSERT_EQ(Slice.StripIdx, 2);
  ASSERT_EQ(Slice.X, DreamGeometry::SliceWidth - 1);
  ASSERT_EQ(Slice.Y, 0);
}

TEST_F(DreamGeometryTest,
       SlicePixelFromPixelId_StripLayer3Sector2_BottomRight) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = DreamGeometry::SliceWidth - 1;
  Wanted.Y = 15;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 1);
  ASSERT_EQ(Slice.StripIdx, 2);
  ASSERT_EQ(Slice.X, DreamGeometry::SliceWidth - 1);
  ASSERT_EQ(Slice.Y, 15);
}

TEST_F(DreamGeometryTest, SlicePixelFromPixelId_StripLayer3Sector2_BottomLeft) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = 0;
  Wanted.Y = 15;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 1);
  ASSERT_EQ(Slice.StripIdx, 2);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 15);
}

TEST_F(DreamGeometryTest, SlicePixelFromPixelId_BottomMost) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 22;
  Wanted.StripIdx = 15;
  Wanted.X = 15;
  Wanted.Y = 15;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  ASSERT_EQ(Slice.SectorIdx, 22);
  ASSERT_EQ(Slice.StripIdx, 15);
  ASSERT_EQ(Slice.X, 15);
  ASSERT_EQ(Slice.Y, 15);
}

TEST_F(DreamGeometryTest, SumoPixelFromSlicePixel_TwoFirstAndLast) {
  auto MakeSlicePixel = [](uint32_t X) { return SlicePixel{0, 0, X, 0}; };

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
TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo6_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 20; // Sumo 6
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 9);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo6_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 19;
  Sumo.Y = 0;
  Sumo.Width = 20; // Sumo 6
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo6_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 19;
  Sumo.Y = 15;
  Sumo.Width = 20; // Sumo 6
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo6_BottomLeft) {
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
TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo5_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 16; // Sumo 5
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 7);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo5_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 15;
  Sumo.Y = 0;
  Sumo.Width = 16; // Sumo 5
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo5_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 15;
  Sumo.Y = 15;
  Sumo.Width = 16; // Sumo 5
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo5_BottomLeft) {
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
TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo4_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 12; // Sumo 4
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 5);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo4_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 11;
  Sumo.Y = 0;
  Sumo.Width = 12; // Sumo 4
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo4_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 11;
  Sumo.Y = 15;
  Sumo.Width = 12; // Sumo 4
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo4_BottomLeft) {
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
TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo3_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 8; // Sumo 3
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 3);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo3_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 7;
  Sumo.Y = 0;
  Sumo.Width = 8; // Sumo 3
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo3_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 7;
  Sumo.Y = 15;
  Sumo.Width = 8; // Sumo 3
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo3_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 8; // Sumo 3
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 3);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 0);
}

TEST_F(DreamGeometryTest, EndCapParamsFromPixelId_Pixel1) {
  uint32_t PixelId = 1;
  EndCapParams EndCap = EndCapParamsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 1);
  ASSERT_EQ(EndCap.Sumo, 6);
  ASSERT_EQ(EndCap.Strip, 1);
  ASSERT_EQ(EndCap.Wire, 16);
  ASSERT_EQ(EndCap.Cassette, 10);
  ASSERT_EQ(EndCap.Counter, 2);
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, EndCapParamsFromPixelId_StripLayer3Sector2_TopLeft) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = 0;
  Wanted.Y = 0;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapParamsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.Sumo, 6);
  ASSERT_EQ(EndCap.Strip, 3);
  ASSERT_EQ(EndCap.Wire, 16);
  ASSERT_EQ(EndCap.Cassette, 10);
  ASSERT_EQ(EndCap.Counter, 2);
}

TEST_F(DreamGeometryTest, EndCapParamsFromPixelId_StripLayer3Sector2_TopRight) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = DreamGeometry::SliceWidth - 1;
  Wanted.Y = 0;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapParamsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.Sumo, 3);
  ASSERT_EQ(EndCap.Strip, 3);
  ASSERT_EQ(EndCap.Wire, 16);
  ASSERT_EQ(EndCap.Cassette, 1);
  ASSERT_EQ(EndCap.Counter, 1);
}

TEST_F(DreamGeometryTest,
       EndCapParamsFromPixelId_StripLayer3Sector2_BottomRight) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = DreamGeometry::SliceWidth - 1;
  Wanted.Y = 15;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapParamsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.Sumo, 3);
  ASSERT_EQ(EndCap.Strip, 3);
  ASSERT_EQ(EndCap.Wire, 1);
  ASSERT_EQ(EndCap.Cassette, 1);
  ASSERT_EQ(EndCap.Counter, 1);
}

TEST_F(DreamGeometryTest, EndCapParamsFromPixelId_Sector3_BottomLeft) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 2;
  Wanted.Y = 15;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapParamsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 3);
  ASSERT_EQ(EndCap.Sumo, 6);
  ASSERT_EQ(EndCap.Strip, 1);
  ASSERT_EQ(EndCap.Wire, 1);
  ASSERT_EQ(EndCap.Cassette, 10);
  ASSERT_EQ(EndCap.Counter, 2);
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest,
       SumoPixelFromStripPlanePixel_Sumo6_TopLeft) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 15;
  StripPlane.CassetteIdx = 9;
  StripPlane.CounterIdx = 1;
  uint32_t SumoId = 6;
  uint32_t SumoWidth = 20;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane, SumoId, SumoWidth);
  ASSERT_EQ(Sumo.X, 0);
  ASSERT_EQ(Sumo.Y, 0);
  ASSERT_EQ(Sumo.Sumo, 6);
  ASSERT_EQ(Sumo.Width, 20);
}

TEST_F(DreamGeometryTest,
       SumoPixelFromStripPlanePixel_Sumo6_TopRight) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 15;
  StripPlane.CassetteIdx = 0;
  StripPlane.CounterIdx = 0;
  uint32_t SumoId = 6;
  uint32_t SumoWidth = 20;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane, SumoId, SumoWidth);
  ASSERT_EQ(Sumo.X, 19);
  ASSERT_EQ(Sumo.Y, 0);
  ASSERT_EQ(Sumo.Sumo, 6);
  ASSERT_EQ(Sumo.Width, 20);
}

TEST_F(DreamGeometryTest,
       SumoPixelFromStripPlanePixel_Sumo6_BottomRight) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 0;
  StripPlane.CassetteIdx = 0;
  StripPlane.CounterIdx = 0;
  uint32_t SumoId = 6;
  uint32_t SumoWidth = 20;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane, SumoId, SumoWidth);
  ASSERT_EQ(Sumo.X, 19);
  ASSERT_EQ(Sumo.Y, 15);
  ASSERT_EQ(Sumo.Sumo, 6);
  ASSERT_EQ(Sumo.Width, 20);
}

TEST_F(DreamGeometryTest,
       SumoPixelFromStripPlanePixel_Sumo6_BottomLeft) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 0;
  StripPlane.CassetteIdx = 9;
  StripPlane.CounterIdx = 1;
  uint32_t SumoId = 6;
  uint32_t SumoWidth = 20;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane, SumoId, SumoWidth);
  ASSERT_EQ(Sumo.X, 0);
  ASSERT_EQ(Sumo.Y, 15);
  ASSERT_EQ(Sumo.Sumo, 6);
  ASSERT_EQ(Sumo.Width, 20);
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest,
       SumoPixelFromStripPlanePixel_Sumo3_TopLeft) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 15;
  StripPlane.CassetteIdx = 3;
  StripPlane.CounterIdx = 1;
  uint32_t SumoId = 3;
  uint32_t SumoWidth = 8;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane, SumoId, SumoWidth);
  ASSERT_EQ(Sumo.X, 0);
  ASSERT_EQ(Sumo.Y, 0);
  ASSERT_EQ(Sumo.Sumo, 3);
  ASSERT_EQ(Sumo.Width, 8);
}

TEST_F(DreamGeometryTest,
       SumoPixelFromStripPlanePixel_Sumo3_TopRight) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 15;
  StripPlane.CassetteIdx = 0;
  StripPlane.CounterIdx = 0;
  uint32_t SumoId = 3;
  uint32_t SumoWidth = 8;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane, SumoId, SumoWidth);
  ASSERT_EQ(Sumo.X, 7);
  ASSERT_EQ(Sumo.Y, 0);
  ASSERT_EQ(Sumo.Sumo, 3);
  ASSERT_EQ(Sumo.Width, 8);
}

TEST_F(DreamGeometryTest,
       SumoPixelFromStripPlanePixel_Sumo3_BottomRight) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 0;
  StripPlane.CassetteIdx = 0;
  StripPlane.CounterIdx = 0;
  uint32_t SumoId = 3;
  uint32_t SumoWidth = 8;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane, SumoId, SumoWidth);
  ASSERT_EQ(Sumo.X, 7);
  ASSERT_EQ(Sumo.Y, 15);
  ASSERT_EQ(Sumo.Sumo, 3);
  ASSERT_EQ(Sumo.Width, 8);
}

TEST_F(DreamGeometryTest,
       SumoPixelFromStripPlanePixel_Sumo3_BottomLeft) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 0;
  StripPlane.CassetteIdx = 3;
  StripPlane.CounterIdx = 1;
  uint32_t SumoId = 3;
  uint32_t SumoWidth = 8;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane, SumoId, SumoWidth);
  ASSERT_EQ(Sumo.X, 0);
  ASSERT_EQ(Sumo.Y, 15);
  ASSERT_EQ(Sumo.Sumo, 3);
  ASSERT_EQ(Sumo.Width, 8);
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo6_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 20;
  Sumo.Sumo = 6;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 0);  
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo6_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 19;
  Sumo.Y = 0;
  Sumo.Width = 20;
  Sumo.Sumo = 6;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 19);
  ASSERT_EQ(Slice.Y, 0);  
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo6_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 19;
  Sumo.Y = 15;
  Sumo.Width = 20;
  Sumo.Sumo = 6;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 19);
  ASSERT_EQ(Slice.Y, 15);  
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo6_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 20;
  Sumo.Sumo = 6;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 0);
  ASSERT_EQ(Slice.Y, 15);  
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo5_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 16;
  Sumo.Sumo = 5;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 0);
  ASSERT_EQ(Slice.Y, 0);  
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo5_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 15;
  Sumo.Y = 0;
  Sumo.Width = 16;
  Sumo.Sumo = 5;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 15);
  ASSERT_EQ(Slice.Y, 0);  
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo5_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 15;
  Sumo.Y = 15;
  Sumo.Width = 16;
  Sumo.Sumo = 5;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 15);
  ASSERT_EQ(Slice.Y, 15);  
}


TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo5_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 16;
  Sumo.Sumo = 5;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20);
  ASSERT_EQ(Slice.Y, 15);  
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo4_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 12;
  Sumo.Sumo = 4;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 16 + 0);
  ASSERT_EQ(Slice.Y, 0);  
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo4_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 11;
  Sumo.Y = 0;
  Sumo.Width = 12;
  Sumo.Sumo = 4;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 16 + 11);
  ASSERT_EQ(Slice.Y, 0);  
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo4_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 11;
  Sumo.Y = 15;
  Sumo.Width = 12;
  Sumo.Sumo = 4;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 16 + 11);
  ASSERT_EQ(Slice.Y, 15);
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo4_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 12;
  Sumo.Sumo = 4;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 16 + 0);
  ASSERT_EQ(Slice.Y, 15);
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo3_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 8;
  Sumo.Sumo = 3;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 16 + 12 + 0);
  ASSERT_EQ(Slice.Y, 0);  
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo3_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 7;
  Sumo.Y = 0;
  Sumo.Width = 8;
  Sumo.Sumo = 3;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 16 + 12 + 7);
  ASSERT_EQ(Slice.Y, 0);  
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo3_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 7;
  Sumo.Y = 15;
  Sumo.Width = 8;
  Sumo.Sumo = 3;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 16 + 12 + 7);
  ASSERT_EQ(Slice.Y, 15);  
}

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo3_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 8;
  Sumo.Sumo = 3;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 16 + 12 + 0);
  ASSERT_EQ(Slice.Y, 15);  
}

//-----------------------------------------------------------------------------

#if 0 // this is disabled due to the lack of primitives needed to build the test
TEST_F(DreamGeometryTest,
       EndCapParamsFromPixelId_StripLayer3Sector2_BottomLeft) {
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = 0;
  Wanted.Y = 15;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapParamsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.Sumo, 6);
  ASSERT_EQ(EndCap.Strip, 3);
  ASSERT_EQ(EndCap.Wire, 1);
  ASSERT_EQ(EndCap.Cassette, 10);
  ASSERT_EQ(EndCap.Counter, 2);
}

TEST_F(DreamGeometryTest, BigLoop) {
  uint32_t TestCount = 0;
  for (uint32_t Sector = 1; Sector <= DreamGeometry::SectorCount; Sector++) {
    for (uint32_t Strip = 1; Strip <= 16; Strip++) {
      for (uint32_t Sumo = 3; Sumo <= 6; Sumo++) {
        uint32_t CassettesPerSumo[6] = {-1, -1, -1, 4, 6, 8, 10};
        uint32_t AccumulatedSumoOffsetInSlice[6] = {-1, -1, -1, 0, 20, 36, 48};
        for (uint32_t Cassette = 3; Cassette <= CassettesPerSumo[Sumo];
             Cassette++) {
          for (uint32_t Counter = 1; Counter <= 2; Counter++) {
            for (uint32_t Wire = 1; Wire <= 16; Wire++) {

              struct EndCapParams {
                uint32_t Sector;
                uint32_t Strip;
                uint32_t Sumo;
                uint32_t Cassette;
                uint32_t Counter;
                uint32_t Wire;
              };
              EndCapParams EndCap;

              struct SlicePixel {
                uint32_t SectorIdx;
                uint32_t StripIdx;
                uint32_t X;
                uint32_t Y;
              };
              SlicePixel Slice;
              Slice.SectorIdx = Sector - 1;
              Slice.StripIdx = Strip - 1;
              uint32_t SumoIdx = Sumo - 3;
              uint32_t SumoOffset = AccumulatedSumoOffsetInSlice[Sumo];
              Slice.X = SumoOffset + 

              uint32_t PixelId = ;
            }
          }
        }
      }
    }
  }
  SlicePixel Wanted = {};
  Wanted.SectorIdx = 1;
  Wanted.StripIdx = 2;
  Wanted.X = 0;
  Wanted.Y = 15;
  uint32_t PixelId = PixelIdFromSlicePixel(Wanted);
  EndCapParams EndCap = EndCapParamsFromPixelId(PixelId);
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.Sumo, 6);
  ASSERT_EQ(EndCap.Strip, 3);
  ASSERT_EQ(EndCap.Wire, 1);
  ASSERT_EQ(EndCap.Cassette, 10);
  ASSERT_EQ(EndCap.Counter, 2);
}

#endif