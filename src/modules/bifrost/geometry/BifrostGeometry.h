// Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file
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
#include <modules/caen/geometry/Calibration.h>
#include <modules/caen/geometry/Geometry.h>
#include <string>
#include <utility>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
class BifrostGeometry : public Geometry {
public:

  BifrostGeometry(Config &CaenConfiguration);

  ///\brief virtual method inherited from base class
  uint32_t calcPixel(DataParser::CaenReadout &Data);

  ///\brief virtual method inherited from base class
  bool validateData(DataParser::CaenReadout &Data);

  /// \brief return the global x-offset for the given identifiers
  /// \param Ring logical ring as defined in the ICD
  /// \param TubeId - identifies a tube triplet
  int xOffset(int Ring, int TubeId);

  /// \brief return the global y-offset for the given identifiers
  /// \param TubeId - identifies a tube triplet
  int yOffset(int TubeId);

  /// \brief return the position along the tube
  /// \param AmpA amplitude A from readout data
  /// \param AmpB amplitude B from readout data
  /// \return tube index (0, 1, 2) and normalised position [0.0 ; 1.0]
  /// or (-1, -1.0) if invalid
  std::pair<int, float> calcTubeAndPos(std::vector<float> &Calib,
    int AmpA, int AmpB);


  const int TubesPerTriplet{3};
  const int TripletsPerRing{15};
  int TubePixellation{100}; ///< Number of pixels along a single He tube.

  const std::pair<int, float> InvalidPos{-1, -1.0};
};
} // namespace Caen
