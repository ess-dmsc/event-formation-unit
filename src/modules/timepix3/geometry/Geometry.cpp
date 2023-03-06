// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from timepix readouts
///
//===----------------------------------------------------------------------===//
#include <modules/timepix3/geometry/Geometry.h>
#include <common/debug/Trace.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

  uint32_t Geometry::calcPixel(DataParser::Timepix3Readout &Data){
    XTRACE(DATA, DEB, "calculating pixel");
    uint16_t col = Data.dcol + Data.pix / 4;
    uint16_t row = Data.spix + (Data.pix & 0x3);

    XTRACE(DATA, DEB, "using ESS Geom to calculate pixel id");
    return ESSGeom->pixel2D(col, row);
  }

  bool Geometry::validateData(DataParser::Timepix3Readout &Data){
    XTRACE(DATA, DEB, "validate data, dcol = %u", Data.dcol);
    return true;
  }


}