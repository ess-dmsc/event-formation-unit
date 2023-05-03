// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
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

#include <caen/geometry/Config.h>
#include <common/JsonFile.h>
#include <common/debug/Trace.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {
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

  /// \brief detector specific loading
  void loadBifrostParameters();
  void loadLokiParameters();

  /// \brief vector of (vector of) polynomial coefficients
  std::vector<std::vector<double>> StrawCalibration;

  struct {
    int64_t *ClampLow;
    int64_t *ClampHigh;
  } Stats;


  ///\todo let's later worry about where to put this and
  /// how to read it.
  struct {
    // Per triplet, so 45 in total
    // so far only 'null' calibration nadonly intervals
    std::vector<std::vector<float>> TripletCalib{
      // Ring 0
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},

      // Ring 1
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},

      // Ring 2
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000},
      {0.000, 0.333, 0.333, 0.667, 0.667, 1.000}
    };
  } BifrostCalibration;

private:
  uint32_t NumberOfStraws{0};  ///< number of straws in the calibration
  uint16_t StrawResolution{0}; ///< resolution along a straw
  uint32_t MaxPixelId{0};      ///< The maximum pixelid in the map

  nlohmann::json root;
};
} // namespace Caen
