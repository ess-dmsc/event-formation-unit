// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from timepix readouts
///
//===----------------------------------------------------------------------===//
#include <common/debug/Trace.h>
#include <modules/timepix3/geometry/Timepix3Geometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

Timepix3Geometry::Timepix3Geometry(uint32_t nx, uint32_t ny, uint32_t nz, uint32_t np)
    : ESSGeometry(nx, ny, nz, np){}

uint32_t Timepix3Geometry::calcPixel(DataParser::Timepix3PixelReadout &Data) {
  XTRACE(DATA, DEB, "calculating pixel");
  uint16_t X = calcX(Data);
  uint16_t Y = calcY(Data);

  XTRACE(DATA, DEB, "using ESS Geom to calculate pixel id from X %u, Y %u", X,
         Y);
  return pixel2D(X, Y);
}

// Calculation and naming (Col and Row) is taken over from CFEL-CMI pymepix
// https://github.com/CFEL-CMI/pymepix/blob/develop/pymepix/processing/logic/packet_processor.py
uint32_t Timepix3Geometry::calcX(DataParser::Timepix3PixelReadout &Data) {
  uint16_t Col = Data.Dcol + Data.Pix / 4;
  return Col;
}

// Calculation and naming (Col and Row) is taken over from CFEL-CMI pymepix
// https://github.com/CFEL-CMI/pymepix/blob/develop/pymepix/processing/logic/packet_processor.py
uint32_t Timepix3Geometry::calcY(DataParser::Timepix3PixelReadout &Data) {
  uint16_t Row = Data.Spix + (Data.Pix & 0x3);
  return Row;
}

///\todo implement this
bool Timepix3Geometry::validateData(DataParser::Timepix3PixelReadout &Data) {
  XTRACE(DATA, DEB, "validate data, dcol = %u", Data.Dcol);
  return true;
}

} // namespace Timepix3