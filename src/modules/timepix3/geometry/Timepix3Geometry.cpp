// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from timepix readouts
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <geometry/Timepix3Geometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

Timepix3Geometry::Timepix3Geometry(uint32_t nx, uint32_t ny, uint32_t nz, uint32_t np)
    : ESSGeometry(nx, ny, nz, np){}

uint32_t Timepix3Geometry::calcPixel(const PixelDataEvent& Data) const {
  XTRACE(DATA, DEB, "calculating pixel");
  uint16_t X = calcX(Data);
  uint16_t Y = calcY(Data);

  XTRACE(DATA, DEB, "using ESS Geom to calculate pixel id from X %u, Y %u", X,
         Y);
  return pixel2D(X, Y);
}

// Calculation and naming (Col and Row) is taken over from CFEL-CMI pymepix
// https://github.com/CFEL-CMI/pymepix/blob/develop/pymepix/processing/logic/packet_processor.py
uint32_t Timepix3Geometry::calcX(const PixelDataEvent& Data) const {
  uint32_t Col = Data.dCol + Data.pix / 4;
  return Col;
}

// Calculation and naming (Col and Row) is taken over from CFEL-CMI pymepix
// https://github.com/CFEL-CMI/pymepix/blob/develop/pymepix/processing/logic/packet_processor.py
uint32_t Timepix3Geometry::calcY(const PixelDataEvent& Data) const {
  uint32_t Row = Data.sPix + (Data.pix & 0x3);
  return Row;
}

///\todo implement this
bool Timepix3Geometry::validateData(const PixelDataEvent& Data) const {
  XTRACE(DATA, DEB, "validate data, dcol = %u", Data.dCol);
  return true;
}

} // namespace Timepix3