/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Common class for calculating pixelid
 */

#pragma once
#include <cinttypes>

class ESSGeometry {
 public:
  /** @brief specify all four dimensions for geometry regardless of
   * detector type.
   * @param nx number of x values
   * @param ny number of y values
   * @param nz number of z values
   * @param np number of panels
   */
  ESSGeometry(uint32_t nx, uint32_t ny, uint32_t nz, uint32_t np) :
      nx_(nx), ny_(ny), nz_(nz), np_(np) {
    precalculate();
  }

  /** @brief default constructor */
  ESSGeometry() {}

  /** @brief set number of x values */
  void nx(uint32_t new_nx) {
    nx_ = new_nx;
    precalculate();
  }

  /** @brief set number of y values */
  void ny(uint32_t new_ny) {
    ny_ = new_ny;
    precalculate();
  }

  /** @brief set number of z values */
  void nz(uint32_t new_nz) {
    nz_ = new_nz;
    precalculate();
  }

  /** @brief set number of panels */
  void np(uint32_t new_np) {
    np_ = new_np;
    precalculate();
  }

  /** @brief get number of x values */
  uint32_t nx() const {
    return nx_;
  }

  /** @brief get number of y values */
  uint32_t ny() const {
    return ny_;
  }

  /** @brief get number of z values */
  uint32_t nz() const {
    return nz_;
  }

  /** @brief get number of panels */
  uint32_t np() const {
    return np_;
  }

  /** @brief check if geometry definition is valid */
  bool valid() const {
    return (0 != nxyzp_);
  }

  /** @brief check if a given a pixel value is valid */
  bool valid_id(uint32_t pixelid) const {
    return (pixelid >= 1) && (pixelid <= nxyzp_);
  }

  /** @brief get the highest pixel value for the detector */
  inline uint32_t max_pixel() const {
    return nxyzp_;
  }

  /** @brief calculate pixelid for a single panel 2D detector */
  inline uint32_t pixel2D(uint32_t x, uint32_t y) const {
    return pixelMP3D(x, y, 0, 0);
  }

  /** @brief calculate pixelid for a multi-panel 2D detector */
  inline uint32_t pixelMP2D(uint32_t x, uint32_t y, uint32_t p) const {
    return pixelMP3D(x, y, 0, p);
  }

  /** @brief calculate pixelid for a single panel 3D detector */
  inline uint32_t pixel3D(uint32_t x, uint32_t y, uint32_t z) const {
    return pixelMP3D(x, y, z, 0);
  }

  /** @brief calculate pixelid for a multi-panel 3D detector */
  inline uint32_t pixelMP3D(uint32_t x, uint32_t y, uint32_t z, uint32_t p) const {
    if ((x >= nx_) || (y >= ny_) || (z >= nz_) || (p >= np_)) {
      return 0; /** 0 is invalid as pixel */
    }
    // Valid values for all coordinates
    return p * nxyz_ + z * nxy_ + y * nx_ + x + 1;
  }

  /** @brief get x coordinate from pixelid */
  inline uint32_t x(uint32_t pixel) const {
    return (pixel - 1) % nx_;
  }

  /** @brief get y coordinate from pixelid */
  inline uint32_t y(uint32_t pixel) const {
    return ((pixel - 1) / nx_) % ny_;
  }

  /** @brief get z coordinate from pixelid */
  inline uint32_t z(uint32_t pixel) const {
    return ((pixel - 1) / nxy_) % nz_;
  }

  /** @brief get p coordinate from pixelid */
  inline uint32_t p(uint32_t pixel) const {
    return (pixel - 1) / nxyz_;
  }

 private:
  uint32_t nx_{0}; /**< number of x segments */
  uint32_t ny_{0}; /**< number of y segments */
  uint32_t nz_{0}; /**< number of z segments */
  uint32_t np_{0}; /**< number of panels */
  uint32_t nxy_{0}; /**< npixels in xy plane */
  uint32_t nxyz_{0}; /**< npixels in xyz voluem */
  uint32_t nxyzp_{0}; /**< total number of pixels */


  /** @brief precompute transormation factors */
  void precalculate()
  {
    nxy_ = nx_ * ny_;
    nxyz_ = nxy_ * nz_;
    nxyzp_ = nxyz_ * np_;
  }
};
