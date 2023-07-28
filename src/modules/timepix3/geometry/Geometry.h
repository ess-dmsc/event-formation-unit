// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from timepix readouts
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/timepix3/readout/DataParser.h>

#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {
class Geometry {
public:
  /// \brief sets the pixel resolution of a straw
  /// \param Resolution integer value to set straw resolution to
  void setXResolution(uint16_t Resolution) { XResolution = Resolution; }
  void setYResolution(uint16_t Resolution) { YResolution = Resolution; }

  /// \brief calculates an integer pixel value from a Timepix3PixelReadout
  /// object \param Data Timepix3PixelReadout object, containing ADC value
  /// information,
  ///         TubeID, and other information needed to determine pixel of
  ///         event. If a Calibration has been set, it will be applied here.
  uint32_t calcPixel(DataParser::Timepix3PixelReadout &Data);

  uint32_t calcX(DataParser::Timepix3PixelReadout &Data);

  uint32_t calcY(DataParser::Timepix3PixelReadout &Data);

  /// \brief returns true if Data is a valid readout with the given config
  /// \param Data Timepix3PixelReadout to check validity of.
  bool validateData(DataParser::Timepix3PixelReadout &Data);

  ESSGeometry *ESSGeom;
  std::uint16_t XResolution; ///< resolution of X axis
  std::uint16_t YResolution; ///< resolution of Y axis
};
} // namespace Timepix3
