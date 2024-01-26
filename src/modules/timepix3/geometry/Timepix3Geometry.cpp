// Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
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

using namespace timepixReadout;

Timepix3Geometry::Timepix3Geometry(uint32_t nx, uint32_t ny,
                                   uint32_t numberOfChunks)
    : ESSGeometry(nx, ny, 1, 1),
      totalNumberOfChunks(numberOfChunks <= 0 ? 1 : numberOfChunks),
      chunksPerDimension(static_cast<int>(sqrt(totalNumberOfChunks))),
      chunkSize(nx / chunksPerDimension) {}

uint32_t Timepix3Geometry::calcPixel(const PixelReadout &Data) const {
  XTRACE(DATA, DEB, "calculating pixel");
  uint16_t X = calcX(Data);
  uint16_t Y = calcY(Data);

  XTRACE(DATA, DEB, "using ESS Geom to calculate pixel id from X %u, Y %u", X,
         Y);
  return pixel2D(X, Y);
}

/// \brief Calculates the chunk window index based on the X and Y coordinates
/// \param X is the X coordinate
/// \param Y is the Y coordinate
/// \return the chunk window index
int Timepix3Geometry::getChunkWindowIndex(const uint16_t X,
                                          const uint16_t Y) const {
  // If there is only one chunk, return 0 index position
  if (totalNumberOfChunks == 1) {
    return 0;
  }

  // Calculate the window index based on the X and Y coordinates
  return (X / chunkSize) + (chunksPerDimension * (Y / chunkSize));
}

// Calculation and naming (Col and Row) is taken over from CFEL-CMI pymepix
// https://github.com/CFEL-CMI/pymepix/blob/develop/pymepix/processing/logic/packet_processor.py
uint32_t Timepix3Geometry::calcX(const PixelReadout &Data) const {
  uint32_t Col = Data.dCol + Data.pix / 4;
  return Col;
}

// Calculation and naming (Col and Row) is taken over from CFEL-CMI pymepix
// https://github.com/CFEL-CMI/pymepix/blob/develop/pymepix/processing/logic/packet_processor.py
uint32_t Timepix3Geometry::calcY(const PixelReadout &Data) const {
  uint32_t Row = Data.sPix + (Data.pix & 0x3);
  return Row;
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