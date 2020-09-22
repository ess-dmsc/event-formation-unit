// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Specifying the geometry of a SoNDe Detector prototype, provides a
/// calculation of global detector pixel id
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/Trace.h>
#include <logical_geometry/ESSGeometry.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Sonde {

class Geometry {
public:
  /// \brief Create a SoNDe Geometry based on number of columns, grid
  Geometry() {}

  /// \brief returns the maximum available pixelid for this geometry
  int getmaxpixelid() { return essgeometry.max_pixel(); }

  /// Sometimes asic and channel are separate \todo make this one the default
  int getdetectorpixelid(int module, int asic, int channel) {
    return getdetectorpixelid(module, (asic << 6) + (channel & 0x3f));
  }

  /// \brief Return the global detector pixel id from
  inline int getdetectorpixelid(int module, int asch) {
    int asic = asch >> 6;
    if (asic > 3) {
      XTRACE(PROCESS, WAR, "Invalid asic: %d", asic);
      return -1;
    }
    int channel = asch & 0x3f;
    if (channel > 15) {
      XTRACE(PROCESS, WAR, "Invalid channel: %d", channel);
      return -1;
    }
    XTRACE(PROCESS, DEB, "module %d, asic %d, channel %d", module, asic,
           channel);

    int x = channel % 4;
    int y = channel / 4;

    XTRACE(PROCESS, DEB, "initial coords x: %d, y %d ", x, y);

    if (asic == 0) {
      x = 7 - x;
      y = 3 - y;
    } else if (asic == 3) {
      y = y + 4;
    } else if (asic == 2) {
      x = 7 - x;
      y = 7 - y;
    }

    int pixelid = essgeometry.pixel2D(x, y);
    XTRACE(PROCESS, DEB, "coordinates: x %d, y %d, pixel_id: %d", x, y,
           pixelid);
    return pixelid;
  }
private:
  ESSGeometry essgeometry{8, 8, 1, 1}; /// 1x1 module has 8x8 pixels
};

}
