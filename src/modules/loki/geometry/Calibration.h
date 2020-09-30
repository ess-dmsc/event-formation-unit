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


  uint16_t strawCorrection(uint16_t Straw, uint16_t Pos) {
    return StrawMapping[Straw][Pos];
  }

  /// \brief the new scheme to be implemented
  std::vector<std::vector<uint16_t>> StrawMapping;

private:
  void addCalibrationEntry(uint32_t Straw, std::vector<float> Polynomial);

  uint32_t NumberOfStraws{0}; ///< number of straws in the calibration
  uint16_t StrawResolution{0}; ///< resolution along a straw
  uint32_t MaxPixelId{0}; ///< The maximum pixelid in the map
};
} // namespace
