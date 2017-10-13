/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Specifying the geometry of a SoNDe Detector, provides
 * a calculation of global detector pixel id
 */

#pragma once
#include <cinttypes>
#include <common/Trace.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

class SoNDeGeometry {
public:
  /** @brief Create a SoNDe Geometry based on number of columns, grid
   */
  SoNDeGeometry() {}

  /** @brief returns the maximum available pixelid for this geometry
   */
  int getmaxpixelid() { return 64; }

  /** @brief Return the global detector pixel id from
   */
  inline int getdetectorpixelid(int module, int asch) {
    int asic = asch >> 6;
    if (asic > 3) {
      XTRACE(PROCESS, WAR, "Invalid asic: %d\n", asic);
      return -1;
    }
    int channel = asch & 0x3f;
    if (channel > 15) {
      XTRACE(PROCESS, WAR, "Invalid channel: %d\n", channel);
      return -1;
    }
    XTRACE(PROCESS, DEB, "module %d, asic %d, channel %d\n", module, asic, channel);

    int x = channel % 4;
    int y = channel / 4;

    XTRACE(PROCESS, DEB, "initial coords x: %d, y %d \n", x,y);

    if (asic == 0) {
        x = 7 - x;
        y = 3 - y;
    } else if (asic == 3) {
        y = y + 4;
    } else if (asic == 2) {
        x = 7 - x;
        y = 7 - y;
    }
    int pixelid = x + 8 * y + 1;
    XTRACE(PROCESS, DEB, "coordinates: x %d, y %d, pixel_id: %d\n", x, y, pixelid);
    return pixelid;
  }
};
