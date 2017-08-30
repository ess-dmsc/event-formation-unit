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
  int getmaxpixelid() { return 42; }

  /** @brief Return the global detector pixel id from
   */
  inline int getdetectorpixelid(int module, int asch) {
    XTRACE(PROCESS, DEB, "module %d, asic %d, channel %d\n", module, asch>>6,
           asch&0x3f);

    return 42; /** @todo perform real calculation */
  }
};
