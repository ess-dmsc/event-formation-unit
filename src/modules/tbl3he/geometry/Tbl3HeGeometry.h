// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calculate pixelid from tube and amplitudes
///
/// Pixel definitions taken from the ICD, the latest version of which
/// can be found through Instrument Status Overview
/// https://confluence.esss.lu.se/display/ECDC/Instrument+Status+Overview
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/Config.h>
#include <modules/caen/geometry/Geometry.h>
#include <string>
#include <utility>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class Tbl3HeGeometry : public Geometry {
public:
  Tbl3HeGeometry(Config &CaenConfiguration);

  ///\brief virtual method inherited from base class
  uint32_t calcPixel(DataParser::CaenReadout &Data);

  ///\brief virtual method inherited from base class
  bool validateData(DataParser::CaenReadout &Data);

  const int UnitsPerGroup{1};
  int UnitPixellation{100}; ///< Number of pixels along a single He tube.

  const std::pair<int, float> InvalidPos{-1, -1.0};
};
} // namespace Caen
