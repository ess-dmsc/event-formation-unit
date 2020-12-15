// Copyright (C) 2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Encoder/decoder from PixelId to the physics EndCap parameters for
///        the DREAM detector.
///
/// \brief PixelIdFromEndCapParams() encodes the PixelId from EndCap parameters
///        The input is checked for validity and true is returned on success.
///
/// \brief EndCapParamsFromPixelId() decodes the EndCap parameters from the
/// PixelId.
///        The input is checked for validity and true is returned on success.
///
//===----------------------------------------------------------------------===//

#include <stdint.h>

#include <common/Assert.h>

namespace DreamGeometry {

/* clang-format off

DREAM logical pixel layout
--------------------------

The logical pixel layout of DREAM is parameterized by Sector columns and Strip rows.

            Sector 1  Sector 2  Sector ...
           +---------+---------+-----
Strip      |         |         |
Layer 1    |         |         |
           |         |         |
           +---------+---------+-----
Strip      |         |         |
Layer 2    |         |         |
           |         |         |
           +---------+---------+-----
Strip      |         |         |
Layer ...  |         |         |
           |         |         |

The pair of Sector and Strip is a Sector Slice. It is a rectangle, which contain
the pixels of four SUMOs.

            Sector Slice
            width 56
            struct SlicePixel
           O===>-------------------------------------------------------------+
           ║   X                                                             |
           v Y                                                               |
           |                                                                 |
height     |                                                                 |
16         |                                                                 |
           |                                                                 |
           +-----------------------------------------------------------------+

With the SUMOs partitioned as:

            SUMO 6                      SUMO 5             SUMO 4     SUMO 3
            width 20                    width 16           width 12   width 8
            struct SumoPixel            struct SumoPixel   struct..   struct..
           O===>-----------------------O===>--------------O===>------O===>---+
           ║   X                       ║   X              ║   X      ║   X   |
           v Y                         v Y                v Y        v Y     |
           |                           |                  |          |       |
height     |                           |                  |          |       |
16         |                         W ^                W ^        W ^     W ^
           |                           ║                  ║          ║       ║
           +-----------------------<===P--------------<===P------<===P---<===P
                                   CC                 CC         CC      CC

Where the two coordinate systems (O, P) are present on each SUMO:

            O: Top-left origin                P: Bottom-Right
            X, Y follows the                  W (wire), CC (Cassette Counter / WireLayer)
            Sector Slice space.               Representats the strip plane of a SUMO voxel grid.
            struct SumoPixel                  struct StripPlanePixel
           O===>--------------+              +------------------+
           ║   X              |              |                  |
           v Y                |              |                  |
           |                  |              |                  |
           |                  |              |                  |
           |                  |              |                W ^
           |                  |              |                  ║
           +------------------+              +--------------<===P
                                                            CC
 clang-format on */

enum Enum : uint32_t {
  SliceWidth = 56,  /// Width of the Slice in pixels.
  SliceHeight = 16, /// Height of the Slice in pixels.
  SectorCount = 23, /// Sectors, left to right, in the logical coordinates.
  TotalWidth = SliceWidth * SectorCount,
  TotalHeight = SliceHeight * 16, /// 16 Strips
  TotalPixels = TotalWidth * TotalHeight
};

// these map Sumo Id (3..6) to various SUMO properties.
static const uint8_t SumoWidths[7] = {0, 0, 0, 8, 12, 16, 20};
static const uint8_t SumoCassetteCount[7] = {0, 0, 0, 4, 6, 8, 10};

struct SlicePixel {
  uint32_t SectorIdx;
  uint32_t StripIdx;
  uint32_t X;
  uint32_t Y;
  bool IsValid() const;
};

struct SumoPixel {
  uint8_t X;
  uint8_t Y;
  uint8_t Width;
  uint8_t Sumo;
  bool IsValid() const;
};

struct StripPlanePixel {
  uint8_t WireIdx;
  uint8_t CassetteIdx;
  uint8_t CounterIdx;
  uint8_t Sumo;
  bool IsValid() const;
};

struct EndCapParams {
  uint32_t Sector;
  uint32_t Sumo;
  uint32_t Strip;
  uint32_t Wire;
  uint32_t Cassette;
  uint32_t Counter;
  bool IsValid() const;
};

inline bool IsPixelIdValid(uint32_t PixelId) {
  if (PixelId < 1) {
    return false;
  }
  if (PixelId > TotalPixels) {
    return false;
  }
  return true;
}

inline bool SlicePixel::IsValid() const {
  if (SectorIdx >= 23) {
    return false;
  }
  if (StripIdx >= 16) {
    return false;
  }
  if (X >= DreamGeometry::SliceWidth) {
    return false;
  }
  if (Y >= DreamGeometry::SliceHeight) {
    return false;
  }
  return true;
}

inline bool SumoPixel::IsValid() const {
  if (Sumo < 3 || Sumo > 6) {
    return false;
  }
  if (Width != DreamGeometry::SumoWidths[Sumo]) {
    return false;
  }
  if (X >= DreamGeometry::SumoWidths[Sumo]) {
    return false;
  }
  if (Y >= DreamGeometry::SliceHeight) {
    return false;
  }
  return true;
}

inline bool StripPlanePixel::IsValid() const {
  if (WireIdx >= DreamGeometry::SliceHeight) {
    return false;
  }
  if (Sumo < 3 || Sumo > 6) {
    return false;
  }
  if (CounterIdx > 1) {
    return false;
  }
  if (CassetteIdx >= DreamGeometry::SumoCassetteCount[Sumo]) {
    return false;
  }
  return true;
}

inline bool EndCapParams::IsValid() const {
  if (Sector < 1 || Sector > 23) {
    return false;
  }
  if (Sumo < 3 || Sumo > 6) {
    return false;
  }
  if (Strip < 1 || Strip > 16) {
    return false;
  }
  if (Wire < 1 || Wire > 16) {
    return false;
  }
  if (Cassette < 1 || Cassette > DreamGeometry::SumoCassetteCount[Sumo]) {
    return false;
  }
  if (Counter < 1 || Counter > 2) {
    return false;
  }
  return true;
}

/// \brief this maps pixelid to the SectorStripSlice.
/// \todo can this be changed to "masking" by doing PixelIdFromSlicePixel() in
/// "reverse"?
SlicePixel SlicePixelFromPixelId(uint32_t PixelId) {
  TestEnvAssertMsg(PixelId > 0 && PixelId < TotalPixels + 1, "Bad PixelId");
  uint32_t PixelIdx = PixelId - 1;
  uint32_t SectorIdx = PixelIdx / SliceWidth;
  uint32_t GlobalY = PixelIdx / TotalWidth;
  SlicePixel Slice;
  Slice.SectorIdx = SectorIdx % SectorCount;
  Slice.StripIdx = GlobalY / SliceHeight;
  Slice.X = PixelIdx % SliceWidth;
  Slice.Y = GlobalY % SliceHeight;
  return Slice;
}

uint32_t PixelIdFromSlicePixel(SlicePixel Slice) {
  TestEnvAssertMsg(Slice.IsValid(), "Bad SlicePixel");
  uint32_t PixelId = 1 + Slice.X + Slice.SectorIdx * (SliceWidth) +
                     Slice.Y * (SliceWidth * SectorCount) +
                     Slice.StripIdx * (SliceWidth * SectorCount * SliceHeight);
  return PixelId;
}

// The layout of the Sumo types along a SliceRow, indexd by X
// 66666666666666666666555555555555555544444444444433333333
SumoPixel SumoPixelFromSlicePixel(SlicePixel Slice) {
  TestEnvAssertMsg(Slice.IsValid(), "Bad SlicePixel");
  struct SliceToSumoProperty {
    uint8_t SumoStartOffsetX;
    uint8_t SumoWidth;
    uint8_t SumoId;
  };
  // clang-format off
  static const SliceToSumoProperty SliceToSumoMap[56 / 4] = {
    {  0, 20, 6 }, {  0, 20, 6 }, {  0, 20, 6 }, {  0, 20, 6 }, { 0, 20, 6}, //  0- 4, SlicePixel.X  0-19: Sumo 6, Cols 1-20
    { 20, 16, 5 }, { 20, 16, 5 }, { 20, 16, 5 }, { 20, 16, 5 },              //  5- 8, SlicePixel.X 20-35: Sumo 5, Cols 1-16
    { 36, 12, 4 }, { 36, 12, 4 }, { 36, 12, 4 },                             //  9-11, SlicePixel.X 36-47: Sumo 4, Cols 1-12
    { 48,  8, 3 }, { 48,  8, 3 },                                            // 12-13, SlicePixel.X 48-55: Sumo 3, Cols 1-8
  };
  // clang-format on
  uint32_t XCompact = Slice.X / 4; // Range reduced -> fewer constants
  SliceToSumoProperty SliceToSumo = SliceToSumoMap[XCompact];

  SumoPixel Sumo;
  Sumo.X = uint8_t(Slice.X - SliceToSumo.SumoStartOffsetX);
  Sumo.Y = Slice.Y;
  Sumo.Width = SliceToSumo.SumoWidth;
  Sumo.Sumo = SliceToSumo.SumoId;
  TestEnvAssertMsg(Sumo.IsValid(), "Bad SumoPixel");
  return Sumo;
}

SlicePixel SlicePixelFromSumoPixel(SumoPixel Sumo, uint32_t SectorIdx,
                                   uint32_t StripIdx) {
  TestEnvAssertMsg(Sumo.IsValid(), "Bad SumoPixel");
  static const uint8_t SumoStartOffsetX[4] = {0, 20, 36, 48};
  uint32_t SumoIdx = 6 - Sumo.Sumo; // Sumo6=0, Sumo3=3
  SlicePixel Slice;
  Slice.X = Sumo.X + SumoStartOffsetX[SumoIdx];
  Slice.Y = Sumo.Y;
  Slice.SectorIdx = SectorIdx;
  Slice.StripIdx = StripIdx;
  TestEnvAssertMsg(Slice.IsValid(), "Bad SlicePixel");
  return Slice;
}

StripPlanePixel StripPlanePixelFromSumoPixel(SumoPixel Sumo) {
  TestEnvAssertMsg(Sumo.IsValid(), "Bad SumoPixel");
  StripPlanePixel StripPlane;
  uint32_t CassetteCounterIdx = Sumo.Width - Sumo.X - 1;
  StripPlane.CassetteIdx = CassetteCounterIdx / 2;
  StripPlane.CounterIdx = CassetteCounterIdx % 2;
  StripPlane.WireIdx = DreamGeometry::SliceHeight - Sumo.Y - 1;
  StripPlane.Sumo = Sumo.Sumo;
  TestEnvAssertMsg(StripPlane.IsValid(), "Bad StripPlanePixel");
  return StripPlane;
}

SumoPixel SumoPixelFromStripPlanePixel(StripPlanePixel StripPlane) {
  TestEnvAssertMsg(StripPlane.IsValid(), "Bad StripPlanePixel");
  uint32_t CassetteCounterIdx =
      StripPlane.CassetteIdx * 2 + StripPlane.CounterIdx;
  SumoPixel Sumo;
  Sumo.Sumo = StripPlane.Sumo;
  Sumo.Width = DreamGeometry::SumoWidths[Sumo.Sumo];
  Sumo.X = Sumo.Width - CassetteCounterIdx - 1;
  Sumo.Y = DreamGeometry::SliceHeight - StripPlane.WireIdx - 1;
  TestEnvAssertMsg(Sumo.IsValid(), "Bad SumoPixel");
  return Sumo;
}

bool EndCapParamsFromPixelId(uint32_t PixelId, EndCapParams &OutEndCap) {
  if (!IsPixelIdValid(PixelId)) {
    return false;
  }
  SlicePixel Slice = SlicePixelFromPixelId(PixelId);
  SumoPixel Sumo = SumoPixelFromSlicePixel(Slice);
  StripPlanePixel StripPlane = StripPlanePixelFromSumoPixel(Sumo);

  OutEndCap.Sector = Slice.SectorIdx + 1;
  OutEndCap.Sumo = Sumo.Sumo;
  OutEndCap.Strip = Slice.StripIdx + 1;
  OutEndCap.Wire = StripPlane.WireIdx + 1;
  OutEndCap.Cassette = StripPlane.CassetteIdx + 1;
  OutEndCap.Counter = StripPlane.CounterIdx + 1;
  TestEnvAssertMsg(OutEndCap.IsValid(), "Bad EndCapParams");
  return true;
}

bool PixelIdFromEndCapParams(EndCapParams EndCap, uint32_t &OutPixelId) {
  if (!EndCap.IsValid()) {
    return false;
  }
  StripPlanePixel StripPlane;
  StripPlane.WireIdx = EndCap.Wire - 1;
  StripPlane.CassetteIdx = EndCap.Cassette - 1;
  StripPlane.CounterIdx = EndCap.Counter - 1;
  StripPlane.Sumo = EndCap.Sumo;
  SumoPixel Sumo = SumoPixelFromStripPlanePixel(StripPlane);

  uint32_t SectorIdx = EndCap.Sector - 1;
  uint32_t StripIdx = EndCap.Strip - 1;
  SlicePixel Slice = SlicePixelFromSumoPixel(Sumo, SectorIdx, StripIdx);

  OutPixelId = PixelIdFromSlicePixel(Slice);
  return true;
}

} // namespace DreamGeometry
