/* Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Caen Boron Coated Straw Tubesfunctions
///
/// Ref: Loki TG3.1 Detectors technology "Boron Coated Straw Tubes for LoKI"
/// Davide Raspino 04/09/2019
///
//===----------------------------------------------------------------------===//

#pragma once
#include <cinttypes>
#include <common/debug/Trace.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/caen/geometry/Calibration.h>
#include <modules/caen/readout/DataParser.h>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_ERR

namespace Caen {

class LokiGeometry {
public:
  /// \brief The four amplitudes measured at certain points in the
  /// Helium tube circuit diagram are used to identify the straw that
  /// detected the neutron and also the position along the straw.
  /// Both of these are calculated at the same time and the result
  /// is stored in the two member variables (StrawId, PosId) if an
  /// invalid input is given the output will be outside the valid
  /// ranges.
  bool calcPositions(std::int16_t AmplitudeA, std::int16_t AmplitudeB,
                     std::int16_t AmplitudeC, std::int16_t AmplitudeD);

  void setResolution(uint16_t Resolution) { NPos = Resolution; }
  void setCalibration(Calibration Calib) { CaenCalibration = Calib; }

  struct Stats {
    uint64_t AmplitudeZero{0};
    uint64_t OutsideRegion{0};
  } Stats;

  uint8_t strawCalc(double straw);
  uint32_t calcPixel(PanelGeometry &Panel, uint8_t FEN,
                     DataParser::CaenReadout &Data);

private:
  const std::uint8_t NStraws{7}; ///< number of straws per tube
  std::uint16_t NPos{512};       ///< resolution of position

public:
  /// holds latest calculated values for straw and position
  /// they will hold out-of-range values if calculation fails
  std::uint8_t StrawId{7};
  double PosVal{512.0};
  Calibration CaenCalibration;
  ESSGeometry *ESSGeom;
};

} // namespace Caen
