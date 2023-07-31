// Copyright (C) 2019 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Caen Boron Coated Straw Tubes functions
///
/// Ref: Loki TG3.1 Detectors technology "Boron Coated Straw Tubes for LoKI"
/// Davide Raspino 04/09/2019
///
/// New terminology for charge division: Groups and Units rather than Tubes
/// and Straws
//===----------------------------------------------------------------------===//

#pragma once
#include <cinttypes>
#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/CDCalibration.h>
#include <modules/caen/geometry/Config.h>
#include <modules/caen/geometry/Geometry.h>
#include <modules/caen/readout/DataParser.h>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_ERR

namespace Caen {

class LokiGeometry : public Geometry {
public:
  LokiGeometry(Config &CaenConfiguration);
  /// \brief The four amplitudes measured at certain points in the
  /// Helium tube circuit diagram are used to identify the straw that
  /// detected the neutron and also the position along the straw.
  /// Both of these are calculated at the same time and the result
  /// is stored in the two member variables (UnitId, PosId) if an
  /// invalid input is given the output will be outside the valid
  /// ranges.
  bool calcPositions(std::int16_t AmplitudeA, std::int16_t AmplitudeB,
                     std::int16_t AmplitudeC, std::int16_t AmplitudeD);

  void setCalibration(CDCalibration Calib) { CaenCDCalibration = Calib; }

  uint8_t getUnitId(double value);
  uint32_t calcPixel(DataParser::CaenReadout &Data);
  bool validateData(DataParser::CaenReadout &Data);

  std::vector<PanelGeometry> &Panels;

  /// holds latest calculated values for straw and position
  /// they will hold out-of-range values if calculation fails
  std::uint8_t UnitId{7};
  double PosVal{1.0};
  const std::uint8_t NUnits{7}; ///< number of straws per tube
  std::vector<double> limits = {0.7, 1.56, 2.52, 3.54, 4.44, 5.3};
};

} // namespace Caen
