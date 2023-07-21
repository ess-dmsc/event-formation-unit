// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
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
#include <modules/caen/geometry/Config.h>
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

  /// \brief return the global x-offset for the given identifiers
  int xOffset(int Ring, int Group);

  /// \brief return local y-coordinate from amplitudes
  int yCoord(int AmpA, int AmpB) { return posAlongUnit(AmpA, AmpB); }

  /// \brief return the position along the unit (tube for CSPEC)
  int posAlongUnit(int AmpA, int AmpB);
};
} // namespace Caen
