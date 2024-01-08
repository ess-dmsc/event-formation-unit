// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from timepix readouts
///
//===----------------------------------------------------------------------===//

#pragma once

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#include "h5cpp/property/dataset_access.hpp"
#include "logical_geometry/ESSGeometry.h"
#include "readout/DataEventTypes.h"
#include <cmath>
#include <cstdint>
#include <math.h>

namespace Timepix3 {

class Timepix3Geometry : public ESSGeometry {

public:
  Timepix3Geometry(uint32_t nx, uint32_t ny, uint32_t numberOfChunks);

  /// \brief sets the pixel resolution of the camera in x plane
  /// \param Resolution integer value to set camera resolution to
  void setXResolution(const uint16_t Resolution) { XResolution = Resolution; }

  /// \brief sets the pixel resolution of the camera in y plane
  /// \param Resolution integer value to set camera resolution to
  void setYResolution(const uint16_t Resolution) { YResolution = Resolution; }

  /// \brief returns the chunk window index in case of partitionning the camera
  /// pixel plane into smaller chunks. This is used in case of parrellel
  /// processing.
  /// \param X and Y coordinates of the pixel
  int getChunkWindowIndex(const uint16_t X, const uint16_t Y) const;

  /// \brief calculates an integer pixel value from a Timepix3PixelReadout
  /// object \param Data Timepix3PixelReadout object
  uint32_t calcPixel(const timepixDTO::PixelDataEvent &Data) const;

  /// \brief returns true if Data is a valid readout with the given config
  /// \param Data PixelDataEvent to check validity of.
  bool validateData(const timepixDTO::PixelDataEvent &Data) const;

  /// \brief calculated the X coordinate from a const PixelDataEvent
  uint32_t calcX(const timepixDTO::PixelDataEvent &Data) const;

  /// \brief calculated the Y coordinate from a const PixelDataEvent
  uint32_t calcY(const timepixDTO::PixelDataEvent &Data) const;

  /// \brief returns the total number of chunks
  int getChunkNumber() const { return totalNumberOfChunks; }

private:
  std::uint16_t XResolution; ///< resolution of X axis
  std::uint16_t YResolution; ///< resolution of Y axis
  int totalNumberOfChunks;   ///< number of chunks windows in total
  int chunksPerDimension;    ///< number of chunks per dimension
  int chunkSize;             ///< size of each chunk
};

} // namespace Timepix3
