/** Copyright (C) 2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the definitions of the ESS Logical geometry
/// The constructor takes four arguments, which are the dimensions in
/// x, y, z, and panel coordinates. Given x, y, z, p (ranges from 0 to dim - 1)
/// methods return valid pixel_id, or 0 if out of range.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdlib>

class ESSGeometry {
public:
  ESSGeometry(size_t dim_x, size_t dim_y, size_t dim_z, size_t num_panels)
           : nx(dim_x), ny(dim_y), nz(dim_z), np(num_panels) {
    nxy = nx * ny;
    nxyz = nxy * nz;
  }

  /// @brief Calculate pixel_id for Single Panel 2D detector
  size_t getPixelSP2D(size_t x, size_t y) {
    if ((x >= nx) || (y >= ny)) {
      return 0;
    }
    return y * nx + x + 1;
  }

  /// @brief Calculate pixel_id for Single Panel 3D detector
  size_t getPixelSP3D(size_t x, size_t y, size_t z) {
    if ((x >= nx) || (y >= ny) || (z >= nz)) {
      return 0;
    }
    return z * nxy + y * nx + x + 1;
  }

  /// @brief Calculate pixel_id for Multi Panel 2D detector
  size_t getPixelMP2D(size_t x, size_t y, size_t p) {
    if ((x >= nx) || (y >= ny) || (p >= np)) {
      return 0;
    }
    return p * nxy + y * nx + x + 1;
  }

  /// @brief Calculate pixel_id for Multi Panel 3D detector
  size_t getPixelMP3D(size_t x, size_t y, size_t z, size_t p) {
    if ((x >= nx) || (y >= ny) || (z >= nz) || (p >= np)) {
      return 0;
    }
    return p * nxyz + z * nxy + y * nx + x + 1;
  }

private:
  size_t nx{1}; /// number of pixels in x
  size_t ny{1}; /// number of pixels in y
  size_t nz{1}; /// number of pixels in z
  size_t np{1}; /// number of panels (number of pixels in p)
  size_t nxy{1}; /// pre calculated product
  size_t nxyz{1}; /// pre calculated product
};
