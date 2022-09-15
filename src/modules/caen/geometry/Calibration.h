/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get tube calibrations from json file
///
/// Calibration method is described in the LoKI ICD which can be found here
/// https://confluence.esss.lu.se/display/ECDC/Instrument+Status+Overview
//===----------------------------------------------------------------------===//

#pragma once

#include <common/debug/Trace.h>
#include <caen/geometry/Config.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {
class Calibration {
public:
  Calibration();

  /// \brief populate the strawcalibration vector with the provided
  /// polynomial coefficients
  Calibration(std::string CalibrationFile);

  /// \brief Create a null 'Calibration' no pixel mapping is done
  void nullCalibration(uint32_t Straws, uint16_t Resolution);

  /// \brief return the maximum pixel id
  uint32_t getMaxPixel() { return MaxPixelId; }

  /// \brief apply the position correction
  uint32_t strawCorrection(uint32_t StrawId, double Pos);

  /// \brief vector of (vector of) polynomial coefficients
  std::vector<std::vector<double>> StrawCalibration;

  struct {
    uint64_t ClampLow{0};
    uint64_t ClampHigh{0};
  } Stats;

private:
  uint32_t NumberOfStraws{0};  ///< number of straws in the calibration
  uint16_t StrawResolution{0}; ///< resolution along a straw
  uint32_t MaxPixelId{0};      ///< The maximum pixelid in the map
};
} // namespace Loki
