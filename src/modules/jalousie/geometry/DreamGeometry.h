#include <stdint.h>

// This is the main PixelId decoder function
struct EndCapParams EndCapParamsFromPixelId(uint32_t PixelId);

/// \todo Handle pixel min/max value test?

// test global pixel to sumo slice mapping

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
           +-----------------------<===X--------------<===X------<===X---<===X
                                   CC                 CC         CC      CC
Legend: 
  Coordinate system O:
    X: Zero 

  W : WireId
  CC: Cassette + Counter (WireLayer)
 
 
 clang-format on */

/// A Slice refers to a Sector Sumo Slice (SSS), which is a slice of a Sector
/// along the Strip axis.
struct SlicePixel {
  uint32_t SectorIdx;
  uint32_t StripIdx;
  uint32_t X;
  uint32_t Y;
};

// Origin top-left, X goes right, Y goes down.
struct SumoPixel {
  uint8_t X;
  uint8_t Y;
  uint8_t Width;
  uint8_t Sumo;
};

struct StripPlanePixel {
  uint32_t WireIdx;
  uint32_t CassetteIdx;
  uint32_t CounterIdx;
};

struct EndCapParams {
  uint32_t Sector;
  uint32_t Sumo;
  uint32_t Strip;
  uint32_t Wire;
  uint32_t Cassette;
  uint32_t Counter;
};

namespace DreamGeometry {
enum Enum : uint32_t {
  SliceWidth = 56,  /// Width of the Slice (SSS) in pixels.
  SliceHeight = 16, /// Height of the Slice (SSS) in pixels.
  SectorCount = 23, /// Sectors, left to right, in the logical coordinates.
  TotalWidth = SliceWidth * SectorCount,
  TotalHeight = SliceHeight * 16, /// 16 Strips
  TotalPixels = TotalWidth * TotalHeight
};
}

/// \brief this maps pixelid to the SectorStripSlice.
/// \todo can this be changed to "masking" by doing PixelIdFromSlicePixel() in
/// "reverse"?
SlicePixel SlicePixelFromPixelId(uint32_t PixelId) {
  using namespace DreamGeometry;
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
  using namespace DreamGeometry;
  uint32_t PixelId = 1 + Slice.X + Slice.SectorIdx * (SliceWidth) +
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
  return Sumo;
}

SlicePixel SlicePixelFromSumoPixel(SumoPixel Sumo, uint32_t SectorIdx,
                                   uint32_t StripIdx) {
  static const uint8_t SumoStartOffsetX[4] = {0, 20, 36, 48};
  uint32_t SumoIdx = 6 - Sumo.Sumo; // Sumo6=0, Sumo3=3
  SlicePixel Slice;
  Slice.X = Sumo.X + SumoStartOffsetX[SumoIdx];
  Slice.Y = Sumo.Y;
  Slice.SectorIdx = SectorIdx;
  Slice.StripIdx = StripIdx;
  return Slice;
}

StripPlanePixel StripPlanePixelFromSumoPixel(SumoPixel Sumo) {
  StripPlanePixel StripPlane;
  uint32_t CassetteCounterIdx = Sumo.Width - Sumo.X - 1;
  StripPlane.CassetteIdx = CassetteCounterIdx / 2;
  StripPlane.CounterIdx = CassetteCounterIdx % 2;
  StripPlane.WireIdx = DreamGeometry::SliceHeight - Sumo.Y - 1;
  return StripPlane;
}

SumoPixel SumoPixelFromStripPlanePixel(StripPlanePixel StripPlane,
                                       uint32_t SumoId, uint32_t SumoWidth) {
  uint32_t CassetteCounterIdx =
      StripPlane.CassetteIdx * 2 + StripPlane.CounterIdx;
  SumoPixel Sumo;
  Sumo.X = SumoWidth - CassetteCounterIdx - 1;
  Sumo.Y = DreamGeometry::SliceHeight - StripPlane.WireIdx - 1;
  Sumo.Width = SumoWidth;
  Sumo.Sumo = SumoId;
  return Sumo;
}

EndCapParams EndCapParamsFromPixelId(uint32_t PixelId) {
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