// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from tube and amplitudes
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Miracles {
class Geometry {
public:
  /// \brief return local x-coordinate from amplitudes
  int xCoord(int Ring, int Tube, int AmpA, int AmpB);

  /// \brief return local y-coordinate from amplitudes
  int yCoord(int Ring, int AmpA, int AmpB);


  int tubeAorB(int AmpA, int AmpB);

  /// \brief return the position along the tube
  int posAlongTube(int AmpA, int AmpB);

private:
  int PosResolution{128}; // covers two tubes, so 64 to each
};
} // namespace Miracles
