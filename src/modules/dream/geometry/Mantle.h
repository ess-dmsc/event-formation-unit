// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CDT Mantle module abstractions
///
/// Consult ICD for logical geometry dimensions, rotations etc.
//===----------------------------------------------------------------------===//

#pragma once

#include <logical_geometry/ESSGeometry.h>

namespace Dream {

class Mantle {
public:

  int getX(int Strip) {
    return Strip;
  }

  /// \brief get global y-coordinate for Cuboid with a given index
  int getY(int MU, int Cassette, int Counter, int Wire) {
    return 120 * Wire + 12 * MU + 2 * Cassette + Counter;
  }


  //
  int getPixelId(int MU, int Cassette, int Counter, int Wire, int Strip) {
    int x = getX(Strip);
    int y = getY(MU, Cassette, Counter, Wire);
    return Geometry.pixel2D(x, y);
  }

private:
  ESSGeometry Geometry{256, 1920, 1, 1};


};
} // namespace
