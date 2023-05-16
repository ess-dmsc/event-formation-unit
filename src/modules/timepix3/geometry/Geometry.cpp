// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from timepix readouts
///
//===----------------------------------------------------------------------===//
#include <common/debug/Trace.h>
#include <modules/timepix3/geometry/Geometry.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

uint32_t Geometry::calcPixel(DataParser::Timepix3PixelReadout &Data) {
  XTRACE(DATA, DEB, "calculating pixel");
  uint16_t Col = Data.Dcol + Data.Pix / 4;
  uint16_t Row = Data.Spix + (Data.Pix & 0x3);

  XTRACE(DATA, DEB, "using ESS Geom to calculate pixel id from Col %u, Row %u", Col, Row);
  return ESSGeom->pixel2D(Col, Row);
}

uint32_t Geometry::calcX(DataParser::Timepix3PixelReadout &Data) {
  uint16_t Col = Data.Dcol + Data.Pix / 4;
  return Col;
}

uint32_t Geometry::calcY(DataParser::Timepix3PixelReadout &Data) {
  uint16_t row = Data.Spix + (Data.Pix & 0x3);
  return row;
}

bool Geometry::validateData(DataParser::Timepix3PixelReadout &Data) {
  XTRACE(DATA, DEB, "validate data, dcol = %u", Data.Dcol);
  return true;
}

} // namespace Timepix3