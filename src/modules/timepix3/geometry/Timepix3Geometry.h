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
class Timepix3Geometry : public ESSGeometry {

public:
  Timepix3Geometry(uint32_t nx, uint32_t ny, uint32_t nz, uint32_t np);

  /// \brief sets the pixel resolution of the camera in x plane
  /// \param Resolution integer value to set camera resolution to
  void setXResolution(uint16_t Resolution) { XResolution = Resolution; }

  /// \brief sets the pixel resolution of the camera in y plane
  /// \param Resolution integer value to set camera resolution to
  void setYResolution(uint16_t Resolution) { YResolution = Resolution; }

  /// \brief calculates an integer pixel value from a Timepix3PixelReadout
  /// object \param Data Timepix3PixelReadout object
  uint32_t calcPixel(Timepix3PixelReadout &Data);

  /// \brief returns true if Data is a valid readout with the given config
  /// \param Data Timepix3PixelReadout to check validity of.
  bool validateData(Timepix3PixelReadout &Data);
  
  /// \brief calculated the X coordinate from a Timepix3PixelReadout
  uint32_t calcX(Timepix3PixelReadout &Data);

  /// \brief calculated the Y coordinate from a Timepix3PixelReadout
  uint32_t calcY(Timepix3PixelReadout &Data);

private:
  std::uint16_t XResolution; ///< resolution of X axis
  std::uint16_t YResolution; ///< resolution of Y axis
};
} // namespace Timepix3
