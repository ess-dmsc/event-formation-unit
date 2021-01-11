/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Get tube calibrations from json file
///
//===----------------------------------------------------------------------===//


#pragma once

#include <common/Trace.h>
#include <loki/geometry/Config.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {
class Calibration {
public:

  Calibration();

  /// \brief Create pixelmappings from calibration file
  /// reads a vector of uint32_t and updated the number of pixels
  Calibration(std::string CalibrationFile);

  /// /brief Create a null 'Calibration' no pixel mapping is done
  void nullCalibration(uint32_t Straws, uint16_t Resolution);

  /// \brief return the maximum pixel id
  uint32_t getMaxPixel() { return MaxPixelId; }


  uint32_t strawCorrection(uint32_t strawId, uint16_t Pos) {
    double pos = (double)Pos;
    double a = StrawCalibration[strawId][0];
    double b = StrawCalibration[strawId][1];
    double c = StrawCalibration[strawId][2];
    double d = StrawCalibration[strawId][3];
    //printf("pos: %g, correction %g\n", pos, -(a + b * pos + c * pos*pos + d * pos*pos*pos));
    double res = pos - (a + b * pos + c * pos*pos + d * pos*pos*pos);
    if (res < 0)
      res = 0;
    if (res > StrawResolution)
      res = StrawResolution;
    return (uint32_t)res;
  }


  /// \brief vector of (vector of) four
  std::vector<std::vector<double>> StrawCalibration;

private:

  uint32_t NumberOfStraws{0}; ///< number of straws in the calibration
  uint16_t StrawResolution{0}; ///< resolution along a straw
  uint32_t MaxPixelId{0}; ///< The maximum pixelid in the map
};
} // namespace
