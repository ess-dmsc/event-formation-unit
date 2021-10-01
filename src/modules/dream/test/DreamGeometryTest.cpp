#include <common/testutils/TestBase.h>

#include <bitset>
#include <common/debug/Trace.h>
#include <dream/geometry/DreamGeometry.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace DreamGeometry;

class DreamGeometryTest : public TestBase {
protected:
  uint32_t calcPixel(uint8_t Sector, uint8_t Sumo, uint8_t Strip, uint8_t Wire,
                     uint8_t Cassette, uint8_t Counter) {
    DreamGeometry::EndCapParams endcap = {Sector, Sumo,     Strip,
                                          Wire,   Cassette, Counter};

    uint32_t Pixel{0};
    PixelIdFromEndCapParams(endcap, Pixel);
    return Pixel;
  }
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DreamGeometryTest, IsPixelIdValid) {
  ASSERT_EQ(IsPixelIdValid(1), true);
  ASSERT_EQ(IsPixelIdValid(DreamGeometry::TotalPixels), true);
  ASSERT_EQ(IsPixelIdValid(0), false);
  ASSERT_EQ(IsPixelIdValid(DreamGeometry::TotalPixels + 1), false);
}

TEST_F(DreamGeometryTest, SlicePixel_IsValid) {
  {
    SlicePixel Slice = {};
    ASSERT_EQ(Slice.IsValid(), true);
  }
  {
    SlicePixel Slice = {};
    Slice.SectorIdx = 23;
    ASSERT_EQ(Slice.IsValid(), false);
  }
  {
    SlicePixel Slice = {};
    Slice.StripIdx = 16;
    ASSERT_EQ(Slice.IsValid(), false);
  }
  {
    SlicePixel Slice = {};
    Slice.X = DreamGeometry::SliceWidth;
    ASSERT_EQ(Slice.IsValid(), false);
  }
  {
    SlicePixel Slice = {};
    Slice.Y = DreamGeometry::SliceHeight;
    ASSERT_EQ(Slice.IsValid(), false);
  }
}

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

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, SumoPixel_IsValid) {
  auto MakeSumoPixel = [](uint32_t SumoId) -> SumoPixel {
    SumoPixel Sumo;
    Sumo.SumoId = SumoId;
    Sumo.Width = DreamGeometry::SumoWidths[SumoId];
    Sumo.X = Sumo.Width - 1;
    Sumo.Y = 15;
    return Sumo;
  };
  // Sumo 3
  {
    SumoPixel Sumo = MakeSumoPixel(3);
    ASSERT_EQ(Sumo.IsValid(), true);
  }
  {
    SumoPixel Sumo = MakeSumoPixel(3);
    Sumo.SumoId = 2;
    ASSERT_EQ(Sumo.IsValid(), false);
  }
  {
    SumoPixel Sumo = MakeSumoPixel(3);
    Sumo.SumoId = 7;
    ASSERT_EQ(Sumo.IsValid(), false);
  }
  {
    SumoPixel Sumo = MakeSumoPixel(3);
    Sumo.Width = 2;
    ASSERT_EQ(Sumo.IsValid(), false);
  }
  {
    SumoPixel Sumo = MakeSumoPixel(3);
    Sumo.Width = 55;
    ASSERT_EQ(Sumo.IsValid(), false);
  }
  {
    SumoPixel Sumo = MakeSumoPixel(3);
    Sumo.X = DreamGeometry::SumoWidths[Sumo.SumoId];
    ASSERT_EQ(Sumo.IsValid(), false);
  }
  {
    SumoPixel Sumo = MakeSumoPixel(3);
    Sumo.Y = DreamGeometry::SliceHeight;
    ASSERT_EQ(Sumo.IsValid(), false);
  }
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

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 00)).SumoId, 6);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 01)).SumoId, 6);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 18)).SumoId, 6);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(0 + 19)).SumoId, 6);

  // Sumo 5
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 00)).X, 00);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 01)).X, 01);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 14)).X, 14);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 15)).X, 15);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 00)).Width, 16);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 01)).Width, 16);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 14)).Width, 16);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 15)).Width, 16);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 00)).SumoId, 5);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 01)).SumoId, 5);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 14)).SumoId, 5);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(20 + 15)).SumoId, 5);

  // Sumo 4
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 00)).X, 00);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 01)).X, 01);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 10)).X, 10);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 11)).X, 11);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 00)).Width, 12);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 01)).Width, 12);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 10)).Width, 12);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 11)).Width, 12);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 00)).SumoId, 4);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 01)).SumoId, 4);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 10)).SumoId, 4);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(36 + 11)).SumoId, 4);

  // Sumo 3
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 0)).X, 0);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 1)).X, 1);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 6)).X, 6);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 7)).X, 7);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 0)).Width, 8);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 1)).Width, 8);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 6)).Width, 8);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 7)).Width, 8);

  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 0)).SumoId, 3);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 1)).SumoId, 3);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 6)).SumoId, 3);
  ASSERT_EQ(SumoPixelFromSlicePixel(MakeSlicePixel(48 + 7)).SumoId, 3);
}

TEST_F(DreamGeometryTest, StripPlanePixel_IsValid) {
  auto MakeStripPlanePixel = [](uint32_t SumoId) -> StripPlanePixel {
    StripPlanePixel StripPlanel;
    StripPlanel.WireIdx = 15;
    StripPlanel.CassetteIdx = DreamGeometry::SumoCassetteCount[SumoId] - 1;
    StripPlanel.CounterIdx = 1;
    StripPlanel.SumoId = SumoId;
    return StripPlanel;
  };

  // Sumo 3
  {
    StripPlanePixel StripPlane = MakeStripPlanePixel(3);
    ASSERT_EQ(StripPlane.IsValid(), true);
  }
  {
    StripPlanePixel StripPlane = MakeStripPlanePixel(3);
    StripPlane.WireIdx = DreamGeometry::SliceHeight;
    ASSERT_EQ(StripPlane.IsValid(), false);
  }
  {
    StripPlanePixel StripPlane = MakeStripPlanePixel(3);
    StripPlane.SumoId = 2;
    ASSERT_EQ(StripPlane.IsValid(), false);
  }
  {
    StripPlanePixel StripPlane = MakeStripPlanePixel(3);
    StripPlane.SumoId = 7;
    ASSERT_EQ(StripPlane.IsValid(), false);
  }
  {
    StripPlanePixel StripPlane = MakeStripPlanePixel(3);
    StripPlane.CounterIdx = 2;
    ASSERT_EQ(StripPlane.IsValid(), false);
  }
  {
    StripPlanePixel StripPlane = MakeStripPlanePixel(3);
    StripPlane.CassetteIdx =
        DreamGeometry::SumoCassetteCount[StripPlane.SumoId];
    ASSERT_EQ(StripPlane.IsValid(), false);
  }
}

//
// Sumo 6
//
TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo6_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 20;
  Sumo.SumoId = 6;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 9);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
  ASSERT_EQ(StripPlane.SumoId, 6);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo6_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 19;
  Sumo.Y = 0;
  Sumo.Width = 20;
  Sumo.SumoId = 6;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
  ASSERT_EQ(StripPlane.SumoId, 6);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo6_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 19;
  Sumo.Y = 15;
  Sumo.Width = 20;
  Sumo.SumoId = 6;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
  ASSERT_EQ(StripPlane.SumoId, 6);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo6_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 20;
  Sumo.SumoId = 6;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 9);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 0);
  ASSERT_EQ(StripPlane.SumoId, 6);
}

//
// Sumo 5
//
TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo5_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 16;
  Sumo.SumoId = 5;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 7);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
  ASSERT_EQ(StripPlane.SumoId, 5);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo5_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 15;
  Sumo.Y = 0;
  Sumo.Width = 16;
  Sumo.SumoId = 5;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
  ASSERT_EQ(StripPlane.SumoId, 5);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo5_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 15;
  Sumo.Y = 15;
  Sumo.Width = 16;
  Sumo.SumoId = 5;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
  ASSERT_EQ(StripPlane.SumoId, 5);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo5_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 16;
  Sumo.SumoId = 5;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 7);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 0);
  ASSERT_EQ(StripPlane.SumoId, 5);
}

//
// Sumo 4
//
TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo4_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 12;
  Sumo.SumoId = 4;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 5);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
  ASSERT_EQ(StripPlane.SumoId, 4);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo4_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 11;
  Sumo.Y = 0;
  Sumo.Width = 12;
  Sumo.SumoId = 4;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
  ASSERT_EQ(StripPlane.SumoId, 4);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo4_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 11;
  Sumo.Y = 15;
  Sumo.Width = 12;
  Sumo.SumoId = 4;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
  ASSERT_EQ(StripPlane.SumoId, 4);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo4_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 12;
  Sumo.SumoId = 4;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 5);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 0);
  ASSERT_EQ(StripPlane.SumoId, 4);
}

//
// Sumo 3
//
TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo3_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 8;
  Sumo.SumoId = 3;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 3);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 15);
  ASSERT_EQ(StripPlane.SumoId, 3);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo3_TopRight) {
  SumoPixel Sumo;
  Sumo.X = 7;
  Sumo.Y = 0;
  Sumo.Width = 8;
  Sumo.SumoId = 3;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 15);
  ASSERT_EQ(StripPlane.SumoId, 3);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo3_BottomRight) {
  SumoPixel Sumo;
  Sumo.X = 7;
  Sumo.Y = 15;
  Sumo.Width = 8;
  Sumo.SumoId = 3;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 0);
  ASSERT_EQ(StripPlane.CounterIdx, 0);
  ASSERT_EQ(StripPlane.WireIdx, 0);
  ASSERT_EQ(StripPlane.SumoId, 3);
}

TEST_F(DreamGeometryTest, StripPlanePixelFromSumoPixel_Sumo3_BottomLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 15;
  Sumo.Width = 8;
  Sumo.SumoId = 3;
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);
  ASSERT_EQ(StripPlane.CassetteIdx, 3);
  ASSERT_EQ(StripPlane.CounterIdx, 1);
  ASSERT_EQ(StripPlane.WireIdx, 0);
  ASSERT_EQ(StripPlane.SumoId, 3);
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, EndCapParams_IsValid) {
  auto MakeEndCapParams = [](uint32_t SumoId) -> EndCapParams {
    EndCapParams EndCap;
    EndCap.Sector = 23;
    EndCap.SumoId = SumoId;
    EndCap.Strip = 16;
    EndCap.Wire = 16;
    EndCap.Cassette = DreamGeometry::SumoCassetteCount[SumoId];
    EndCap.Counter = 1;
    return EndCap;
  };

  {
    EndCapParams EndCap = MakeEndCapParams(3);
    ASSERT_EQ(EndCap.IsValid(), true);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.Sector = 24;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.Sector = 0;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.SumoId = 2;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.SumoId = 7;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.Strip = 0;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.Strip = 17;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.Wire = 0;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.Wire = 17;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.Cassette = 0;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.Cassette = DreamGeometry::SumoCassetteCount[EndCap.SumoId] + 1;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.Counter = 0;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
  {
    EndCapParams EndCap = MakeEndCapParams(3);
    EndCap.Counter = 3;
    ASSERT_EQ(EndCap.IsValid(), false);
  }
}

TEST_F(DreamGeometryTest, EndCapParamsFromPixelId_BadPixel) {
  uint32_t PixelId = 0;
  EndCapParams EndCap;
  ASSERT_FALSE(EndCapParamsFromPixelId(PixelId, EndCap));
}

TEST_F(DreamGeometryTest, EndCapParamsFromPixelId_Pixel1) {
  uint32_t PixelId = 1;
  EndCapParams EndCap;
  ASSERT_TRUE(EndCapParamsFromPixelId(PixelId, EndCap));
  ASSERT_EQ(EndCap.Sector, 1);
  ASSERT_EQ(EndCap.SumoId, 6);
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
  EndCapParams EndCap;
  ASSERT_TRUE(EndCapParamsFromPixelId(PixelId, EndCap));
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.SumoId, 6);
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
  EndCapParams EndCap;
  ASSERT_TRUE(EndCapParamsFromPixelId(PixelId, EndCap));
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.SumoId, 3);
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
  EndCapParams EndCap;
  ASSERT_TRUE(EndCapParamsFromPixelId(PixelId, EndCap));
  ASSERT_EQ(EndCap.Sector, 2);
  ASSERT_EQ(EndCap.SumoId, 3);
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
  EndCapParams EndCap;
  ASSERT_TRUE(EndCapParamsFromPixelId(PixelId, EndCap));
  ASSERT_EQ(EndCap.Sector, 3);
  ASSERT_EQ(EndCap.SumoId, 6);
  ASSERT_EQ(EndCap.Strip, 1);
  ASSERT_EQ(EndCap.Wire, 1);
  ASSERT_EQ(EndCap.Cassette, 10);
  ASSERT_EQ(EndCap.Counter, 2);
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, SumoPixelFromStripPlanePixel_Sumo6_TopLeft) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 15;
  StripPlane.CassetteIdx = 9;
  StripPlane.CounterIdx = 1;
  StripPlane.SumoId = 6;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane);
  ASSERT_EQ(Sumo.X, 0);
  ASSERT_EQ(Sumo.Y, 0);
  ASSERT_EQ(Sumo.SumoId, 6);
  ASSERT_EQ(Sumo.Width, 20);
}

TEST_F(DreamGeometryTest, SumoPixelFromStripPlanePixel_Sumo6_TopRight) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 15;
  StripPlane.CassetteIdx = 0;
  StripPlane.CounterIdx = 0;
  StripPlane.SumoId = 6;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane);
  ASSERT_EQ(Sumo.X, 19);
  ASSERT_EQ(Sumo.Y, 0);
  ASSERT_EQ(Sumo.SumoId, 6);
  ASSERT_EQ(Sumo.Width, 20);
}

TEST_F(DreamGeometryTest, SumoPixelFromStripPlanePixel_Sumo6_BottomRight) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 0;
  StripPlane.CassetteIdx = 0;
  StripPlane.CounterIdx = 0;
  StripPlane.SumoId = 6;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane);
  ASSERT_EQ(Sumo.X, 19);
  ASSERT_EQ(Sumo.Y, 15);
  ASSERT_EQ(Sumo.SumoId, 6);
  ASSERT_EQ(Sumo.Width, 20);
}

TEST_F(DreamGeometryTest, SumoPixelFromStripPlanePixel_Sumo6_BottomLeft) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 0;
  StripPlane.CassetteIdx = 9;
  StripPlane.CounterIdx = 1;
  StripPlane.SumoId = 6;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane);
  ASSERT_EQ(Sumo.X, 0);
  ASSERT_EQ(Sumo.Y, 15);
  ASSERT_EQ(Sumo.SumoId, 6);
  ASSERT_EQ(Sumo.Width, 20);
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, SumoPixelFromStripPlanePixel_Sumo3_TopLeft) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 15;
  StripPlane.CassetteIdx = 3;
  StripPlane.CounterIdx = 1;
  StripPlane.SumoId = 3;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane);
  ASSERT_EQ(Sumo.X, 0);
  ASSERT_EQ(Sumo.Y, 0);
  ASSERT_EQ(Sumo.SumoId, 3);
  ASSERT_EQ(Sumo.Width, 8);
}

TEST_F(DreamGeometryTest, SumoPixelFromStripPlanePixel_Sumo3_TopRight) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 15;
  StripPlane.CassetteIdx = 0;
  StripPlane.CounterIdx = 0;
  StripPlane.SumoId = 3;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane);
  ASSERT_EQ(Sumo.X, 7);
  ASSERT_EQ(Sumo.Y, 0);
  ASSERT_EQ(Sumo.SumoId, 3);
  ASSERT_EQ(Sumo.Width, 8);
}

TEST_F(DreamGeometryTest, SumoPixelFromStripPlanePixel_Sumo3_BottomRight) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 0;
  StripPlane.CassetteIdx = 0;
  StripPlane.CounterIdx = 0;
  StripPlane.SumoId = 3;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane);
  ASSERT_EQ(Sumo.X, 7);
  ASSERT_EQ(Sumo.Y, 15);
  ASSERT_EQ(Sumo.SumoId, 3);
  ASSERT_EQ(Sumo.Width, 8);
}

TEST_F(DreamGeometryTest, SumoPixelFromStripPlanePixel_Sumo3_BottomLeft) {
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = 0;
  StripPlane.CassetteIdx = 3;
  StripPlane.CounterIdx = 1;
  StripPlane.SumoId = 3;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane);
  ASSERT_EQ(Sumo.X, 0);
  ASSERT_EQ(Sumo.Y, 15);
  ASSERT_EQ(Sumo.SumoId, 3);
  ASSERT_EQ(Sumo.Width, 8);
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, SlicePixelFromSumoPixel_Sumo6_TopLeft) {
  SumoPixel Sumo;
  Sumo.X = 0;
  Sumo.Y = 0;
  Sumo.Width = 20;
  Sumo.SumoId = 6;
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
  Sumo.SumoId = 6;
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
  Sumo.SumoId = 6;
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
  Sumo.SumoId = 6;
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
  Sumo.SumoId = 5;
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
  Sumo.SumoId = 5;
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
  Sumo.SumoId = 5;
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
  Sumo.SumoId = 5;
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
  Sumo.SumoId = 4;
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
  Sumo.SumoId = 4;
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
  Sumo.SumoId = 4;
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
  Sumo.SumoId = 4;
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
  Sumo.SumoId = 3;
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
  Sumo.SumoId = 3;
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
  Sumo.SumoId = 3;
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
  Sumo.SumoId = 3;
  uint32_t SectorIdx = 0;
  uint32_t StripIdx = 0;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);
  ASSERT_EQ(Slice.SectorIdx, 0);
  ASSERT_EQ(Slice.StripIdx, 0);
  ASSERT_EQ(Slice.X, 20 + 16 + 12 + 0);
  ASSERT_EQ(Slice.Y, 15);
}

TEST_F(DreamGeometryTest, PixelIdFromEndCapParams_BadEndCap) {
  EndCapParams EndCap = {};
  ASSERT_EQ(EndCap.IsValid(), false);
  uint32_t PixelId;
  ASSERT_EQ(PixelIdFromEndCapParams(EndCap, PixelId), false);
}

//-----------------------------------------------------------------------------

TEST_F(DreamGeometryTest, EncodeDecodeAllPixels) {

  std::bitset<DreamGeometry::TotalPixels + 1>
      VisitedPixels; // +1 since we start at 1.

  uint32_t StripStart = 1;
  uint32_t StripLast = 16;
  for (uint32_t Strip = StripStart; Strip <= StripLast; Strip++) {

    uint32_t SectorStart = 1;
    uint32_t SectorLast = DreamGeometry::SectorCount;
    for (uint32_t Sector = SectorStart; Sector <= SectorLast; Sector++) {

      uint32_t SumoIdStart = 3;
      uint32_t SumoIdLast = 6;
      for (uint32_t SumoId = SumoIdStart; SumoId <= SumoIdLast; SumoId++) {

        uint32_t WireStart = 1;
        uint32_t WireLast = 16;
        for (uint32_t Wire = WireStart; Wire <= WireLast; Wire++) {

          uint32_t CassetteStart = 1;
          uint32_t CassetteLast = DreamGeometry::SumoCassetteCount[SumoId];
          for (uint32_t Cassette = CassetteStart; Cassette <= CassetteLast;
               Cassette++) {

            uint32_t CounterStart = 1;
            uint32_t CounterLast = 2;
            for (uint32_t Counter = CounterStart; Counter <= CounterLast;
                 Counter++) {

              EndCapParams SyntheticEndCap;
              SyntheticEndCap.Sector = Sector;
              SyntheticEndCap.SumoId = SumoId;
              SyntheticEndCap.Strip = Strip;
              SyntheticEndCap.Wire = Wire;
              SyntheticEndCap.Cassette = Cassette;
              SyntheticEndCap.Counter = Counter;

              uint32_t PixelId;
              ASSERT_TRUE(PixelIdFromEndCapParams(SyntheticEndCap, PixelId));
              // printf("sect %u, sumo %u, strip %u, wire %u, cassette %u,
              // counter %u -- pixel %u\n",
              //   Sector, SumoId, Strip, Wire, Cassette, Counter, PixelId);
              EndCapParams DecodedEndCap;
              ASSERT_TRUE(EndCapParamsFromPixelId(PixelId, DecodedEndCap));

              // test synthetic endcap and decoded are the same
              ASSERT_EQ(SyntheticEndCap.Sector, DecodedEndCap.Sector);
              ASSERT_EQ(SyntheticEndCap.SumoId, DecodedEndCap.SumoId);
              ASSERT_EQ(SyntheticEndCap.Strip, DecodedEndCap.Strip);
              ASSERT_EQ(SyntheticEndCap.Wire, DecodedEndCap.Wire);
              ASSERT_EQ(SyntheticEndCap.Cassette, DecodedEndCap.Cassette);
              ASSERT_EQ(SyntheticEndCap.Counter, DecodedEndCap.Counter);

              // test we only visit every pixel once
              ASSERT_EQ(VisitedPixels[PixelId], false);
              VisitedPixels[PixelId] = true;
            }
          }
        }
      }
    }
  }

  // test we visit all pixels
  ASSERT_EQ(VisitedPixels.count(), DreamGeometry::TotalPixels);
}

// this tests that test-only asserts assert
TEST_F(DreamGeometryTest, SlicePixelFromPixelId_TestEnvInput) {
  ASSERT_DEATH({ SlicePixelFromPixelId(0); }, "Bad PixelId");
}

TEST_F(DreamGeometryTest, TestingICDDefinitions) {
  //                  sec su  st wi  mo ctr
  ASSERT_EQ(calcPixel(1, 6, 1, 16, 10, 2), 1);    // upper left (z = 0)
  ASSERT_EQ(calcPixel(1, 3, 1, 16, 1, 1), 56);    // next sector 'upper left'
  ASSERT_EQ(calcPixel(23, 3, 1, 16, 1, 1), 1288); // upper right

  ASSERT_EQ(calcPixel(1, 6, 2, 16, 10, 2),
            1288 * 16 + 1); // (x,y,z) = (0, 16, 1)

  ASSERT_EQ(calcPixel(1, 6, 16, 1, 10, 2), 1288 * 255 + 1); // bottom left
  ASSERT_EQ(calcPixel(23, 3, 16, 1, 1, 1), 1288 * 256);     // bottom right
}
