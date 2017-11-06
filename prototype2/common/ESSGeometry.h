/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Common class for calculating pixelid
 */

#pragma once
#include <cinttypes>

class ESSGeometry {
  public:
    ESSGeometry(uint32_t nx, uint32_t ny, uint32_t nz, uint32_t np) :
      nx_(nx), ny_(ny), nz_(nz), np_(np) {
        nxy_ = nx_ * ny_;
        nxyz_= nxy_ * nz;
        nxyzp_ = nxyz_ * np_;
    };

    bool isvalidpixel(uint32_t pixelid) {
      return (pixelid >= 1) && (pixelid <= nxyzp_);
    }

    inline uint32_t getmaxpixel() {
      return nxyzp_;
    }

    inline uint32_t pixelSP2D(uint32_t x, uint32_t y) {
      return pixelMP3D(x, y, 0, 0);
    }

    inline uint32_t pixelMP2D(uint32_t x, uint32_t y, uint32_t p) {
      return pixelMP3D(x, y, 0, p);
    }

    inline uint32_t pixelSP3D(uint32_t x, uint32_t y, uint32_t z) {
      return pixelMP3D(x, y, z, 0);
    }

    inline uint32_t pixelMP3D(uint32_t x, uint32_t y, uint32_t z, uint32_t p) {
      if ((x >= nx_) || (y >= ny_) || (z >= nz_) || (p >= np_)) {
        return 0; /** 0 is invalid as pixel */
      }
      // Valid values for all coordinates
      return p * nxyz_ + z * nxy_ + y * nx_ + x + 1;
    };

 private:
   uint32_t nx_{0}; /**< number of x segments */
   uint32_t ny_{0}; /**< number of y segments */
   uint32_t nz_{0}; /**< number of z segments */
   uint32_t np_{0}; /**< number of panels */
   uint32_t nxy_{0}; /**< npixels in xy plane */
   uint32_t nxyz_{0}; /**< npixels in xyz voluem */
   uint32_t nxyzp_{0}; /**< total number of pixels */
 };
