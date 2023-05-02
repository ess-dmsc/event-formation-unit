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
#include <modules/caen/geometry/Calibration.h>
#include <modules/caen/geometry/Geometry.h>
#include <string>
#include <utility>
#include <vector>
//
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class BifrostGeometry : public Geometry {
public:
  BifrostGeometry(Config &CaenConfiguration);
  uint32_t calcPixel(DataParser::CaenReadout &Data);
  bool validateData(DataParser::CaenReadout &Data);

  int TripletTubes{3}; // tubes per triplet (might be obvious from the name)

  /// \brief return the global x-offset for the given identifiers
  int xOffset(int Ring, int TubeId);

  /// \brief return the global y-offset for the given identifiers
  int yOffset(int TubeId);

  /// \brief return local x-coordinate from amplitudes
  int xCoord(int LocalTube, float UnitPos) {
    if (LocalTube == 1) { // middle tube reversed
      return (TubePixellation - 1) * (1.0 - UnitPos);
    } else {
      return (TubePixellation - 1) * UnitPos;
    }
  }


  int yCoord(int LocalTube) {
    return LocalTube;
  }

  /// \brief return the position along the tube
  /// \return tube index (0, 1, 2) and normalised position [0.0 ; 1.0]
  /// or (-01, -1.0) if invalid
  std::pair<int, float> calcTubeAndPos(int AmpA, int AmpB);

  /// Number of pixels along a He tube.
  int TubePixellation{100};
};
} // namespace Caen
