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
#include <modules/caen/geometry/Geometry.h>
#include <string>
#include <vector>
//
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class CspecGeometry : public Geometry {
public:
    CspecGeometry(Config &CaenConfiguration);
  uint32_t calcPixel(DataParser::CaenReadout &Data);
  bool validateData(DataParser::CaenReadout &Data);

  int PackTubes{24}; // tubes per triplet (might be obvious from the name)

  /// \brief return the global x-offset for the given identifiers
  int xOffset(int Ring, int Tube);

  /// \brief return the global y-offset for the given identifiers
  int yOffset(int Tube);

  /// \brief return local x-coordinate from amplitudes
  int xCoord(int AmpA, int AmpB) {
    int Coord = reverse(posAlongTube(AmpA, AmpB)) % (NPos / 3);
    XTRACE(DATA, DEB, "AmpA %d, AmpB %d, xCoord %d", AmpA, AmpB, Coord);
    return Coord;
  }

  /// \brief return local y-coordinate from amplitudes
  int yCoord(int AmpA, int AmpB) {
    return (reverse(posAlongTube(AmpA, AmpB)) * PackTubes) / NPos;
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
};
} // namespace Caen
