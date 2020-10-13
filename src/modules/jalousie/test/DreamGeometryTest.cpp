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