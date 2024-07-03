// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
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

#include <cmath>
#include <cstdint>
#include <dto/TimepixDataTypes.h>
#include <h5cpp/property/dataset_access.hpp>
#include <logical_geometry/ESSGeometry.h>
#include <math.h>

namespace Timepix3 {

class Timepix3Geometry : public ESSGeometry {

public:
  Timepix3Geometry(uint32_t nx, uint32_t ny, uint8_t scaleUpFactor,
                   uint32_t numChunkWindows);

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

  /// \brief calculates an integer pixel value from a double X and Y coordinate
  /// after clustering
  /// \param X is the X coordinate
  /// \param Y is the Y coordinate
  uint32_t calcPixelId(const double &X, const double &Y) const;

  /// \brief returns true if Data is a valid readout with the given config
  /// \param Data PixelDataEvent to check validity of.
  bool validateData(const timepixReadout::PixelReadout &Data) const;

  /// \brief calculated the X coordinate from a const PixelDataEvent
  // Calculation and naming (Col and Row) is taken over from CFEL-CMI pymepix
  // https://github.com/CFEL-CMI/pymepix/blob/develop/pymepix/processing/logic/packet_processor.py
  static inline uint32_t calcX(const timepixReadout::PixelReadout &Data) {
    uint32_t Col = static_cast<uint32_t>(Data.dCol) + Data.pix / 4;
    return Col;
  }

  /// \brief calculated the Y coordinate from a const PixelDataEvent
  // Calculation and naming (Col and Row) is taken over from CFEL-CMI pymepix
  // https://github.com/CFEL-CMI/pymepix/blob/develop/pymepix/processing/logic/packet_processor.py
  static inline uint32_t calcY(const timepixReadout::PixelReadout &Data) {
    uint32_t Row = static_cast<uint32_t>(Data.sPix) + (Data.pix & 0x3);
    return Row;
  }

  /// \brief returns the total number of chunks
  int getChunkNumber() const { return totalNumChunkWindows; }

private:
  std::uint16_t XResolution;  ///< resolution of X axis
  std::uint16_t YResolution;  ///< resolution of Y axis
  std::uint8_t scaleUpFactor; ///< scale up factor for super resolution
  int totalNumChunkWindows;   ///< number of chunks windows in total
  int chunksPerDimension;     ///< number of chunks per dimension
  int chunkSize;              ///< size of each chunk
};

} // namespace Timepix3
