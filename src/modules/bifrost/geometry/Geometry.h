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

namespace Bifrost {
class Geometry {
public:
  /// \brief return the global x-offset for the given identifiers
  int xOffset(int Ring, int Tube);

  /// \brief return the global y-offset for the given identifiers
  int yOffset(int Tube);

  /// \brief return local x-coordinate from amplitudes
  int xCoord(int AmpA, int AmpB) {
    return reverse(posAlongTube(AmpA, AmpB)) % (PosResolution / 3);
  }

  /// \brief return local y-coordinate from amplitudes
  int yCoord(int AmpA, int AmpB) {
    return (reverse(posAlongTube(AmpA, AmpB)) * TripletTubes) / PosResolution;
  }

  /// \brief return the position along the tube
  int posAlongTube(int AmpA, int AmpB);

  /// NullCalibration just reverses the middle position
  int reverse(int OrgPos) {
    if ((OrgPos < 100) or (OrgPos >= 200)) {
      return OrgPos;
    }
    return 299 - OrgPos;
  }

private:
  int PosResolution{300}; // covers three tubes, so each tube has 100 pixels
  int TripletTubes{3};    // tubes per triplet (might be obvious from the name)
};
} // namespace Bifrost
