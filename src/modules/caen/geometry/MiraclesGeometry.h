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
#include <modules/caen/geometry/Geometry.h>
#include <modules/caen/readout/DataParser.h>


// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class MiraclesGeometry : public Geometry {
public:
  MiraclesGeometry(Config &CaenConfiguration);
  uint32_t calcPixel(DataParser::CaenReadout &Data);
  bool validateData(DataParser::CaenReadout &Data);

  /// \brief return local x-coordinate from amplitudes
  int xCoord(int Ring, int Tube, int AmpA, int AmpB);

  /// \brief return local y-coordinate from amplitudes
  int yCoord(int Ring, int AmpA, int AmpB);


  int tubeAorB(int AmpA, int AmpB);

  /// \brief return the position along the tube
  int posAlongTube(int AmpA, int AmpB);

};
} // namespace Caen
