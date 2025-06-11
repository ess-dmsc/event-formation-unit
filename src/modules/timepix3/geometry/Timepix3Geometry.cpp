// Copyright (C) 2023 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from timepix readouts
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <cstdint>
#include <geometry/Timepix3Geometry.h>
#include <sys/types.h>
#include <utility>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

using namespace timepixReadout;

Timepix3Geometry::Timepix3Geometry(uint32_t nx, uint32_t ny,
                                   uint8_t scaleUpFactor,
                                   uint32_t numChunkWindows)
    : ESSGeometry(scaleUpFactor * nx, scaleUpFactor * ny, 1, 1),
      scaleUpFactor(scaleUpFactor),
      totalNumChunkWindows(numChunkWindows <= 0 ? 1 : numChunkWindows),
      chunksPerDimension(static_cast<int>(sqrt(totalNumChunkWindows))),
      chunkSize(nx / chunksPerDimension) {}

/// \todo: remove this function only used makes unnecessary to have it
uint32_t Timepix3Geometry::calcPixelId(double X, double Y) const {
  XTRACE(
      DATA, DEB,
      "Calculating scaled up pixel coordinates from X %f, Y %f with factor: %d",
      X, Y, scaleUpFactor);
  uint16_t UpScaledX = static_cast<uint16_t>(X * scaleUpFactor);
  uint16_t UpScaledY = static_cast<uint16_t>(Y * scaleUpFactor);

  XTRACE(DATA, DEB, "Calculate pixeId from Scaled X %u, Scaled Y %u", UpScaledX,
         UpScaledY);

  return pixel2D(std::move(UpScaledX), std::move(UpScaledY));
}

/// \brief Calculates the chunk window index based on the X and Y coordinates
/// \param X is the X coordinate
/// \param Y is the Y coordinate
/// \return the chunk window index
int Timepix3Geometry::getChunkWindowIndex(uint16_t X, uint16_t Y) const {
  // If there is only one chunk, return 0 index position
  if (totalNumChunkWindows == 1) {
    return 0;
  }

  // Calculate the window index based on the X and Y coordinates
  return (X / chunkSize) + (chunksPerDimension * (Y / chunkSize));
}

/// \brief Calculates that the received data is fir to the geometry
/// \param Data is the pixel data event
/// \return true if the data is valid
bool Timepix3Geometry::validateData(const PixelReadout &Data) const {

  if (calcX(Data) >= nx()) {
    XTRACE(DATA, WAR, "X value %u is larger than nx limit %u", calcX(Data),
           nx());
    return false;
  } else if (calcY(Data) >= ny()) {
    XTRACE(DATA, WAR, "Y value %u is larger than ny limit %u", calcY(Data),
           ny());
    return false;
  }

  return true;
}

} // namespace Timepix3