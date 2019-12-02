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
  void nullCalibration(uint32_t MaxPixels);

  /// \brief return the maximum pixel id
  uint32_t getMaxPixel() { return MaxPixelId; }


  std::vector<uint32_t> Mapping; ///< The calibration as a mapping between pixels
private:
  uint32_t MaxPixelId{0}; ///< The maximum pixelid in the map
};
} // namespace
