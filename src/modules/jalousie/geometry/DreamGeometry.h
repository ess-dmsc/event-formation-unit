#include <stdint.h>

// This is the main PixelId decoder function
struct EndCapParams EndCapParamsFromPixelId(uint32_t PixelId);

/// \todo Handle pixel min/max value test?

// test global pixel to sumo slice mapping

/// A Slice refers to a Sector Sumo Slice (SSS), which is a slice of a Sector
/// along the Strip axis.
struct SlicePixel {
  uint32_t SectorIdx;
  uint32_t StripIdx;
  uint32_t X;
  uint32_t Y;
};

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
};

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
  static const SliceToSumoProperty SliceToSumoMap[14] = {
    {  0, 20, 6 }, {  0, 20, 6 }, {  0, 20, 6 }, {  0, 20, 6 }, { 0, 20, 6}, //  0- 4, SlicePixel.X  0-19: Sumo 6, Cols 1-20
    { 20, 16, 5 }, { 20, 16, 5 }, { 20, 16, 5 }, { 20, 16, 5 },              //  5- 8, SlicePixel.X 20-35: Sumo 5, Cols 1-16
    { 36, 12, 4 }, { 36, 12, 4 }, { 36, 12, 4 },                             //  9-11, SlicePixel.X 36-47: Sumo 4, Cols 1-12
    { 48,  8, 3 }, { 48,  8, 3 },                                            // 12-13, SlicePixel.X 48-55: Sumo 3, Cols 1-8
  };
  // clang-format on
  uint32_t XCompact =
      Slice.X / 4; // Range reduced from 56 to 14 -> fewer constants
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
  uint32_t CassetteCounterIdx = Sumo.Width - Sumo.X - 1;
  StripPlane.CassetteIdx = CassetteCounterIdx / 2;
  StripPlane.CounterIdx = CassetteCounterIdx % 2;
  StripPlane.WireIdx = DreamGeometry::SliceHeight - Sumo.Y - 1;
  return StripPlane;
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