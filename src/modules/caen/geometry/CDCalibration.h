// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Common abstraction for Charge Division calibration
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <string>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

class CDCalibration {
public:
  CDCalibration(std::string Name) : Name(Name) { };

  /// \brief load json from file into the jsion root object
  CDCalibration(std::string Name, std::string CalibrationFile);

  /// \brief parse the calibration and validate internal consistency
  /// \retval True if file is valid, else False.
  void parseCalibration();

  /// \brief Create a null 'Calibration' no pixel mapping is done
  //void nullCalibration();

  /// \brief return the maximum pixel id
  uint32_t getMaxPixel() { return MaxPixelId; }

  /// \brief apply the position correction
  //uint32_t strawCorrection(uint32_t StrawId, double Pos);


  /// \brief vector of (vector of) polynomial coefficients
  std::vector<std::vector<double>> StrawCalibration;

  // Grafana Counters
  // struct {
  //   int64_t ClampLow;
  //   int64_t ClampHigh;
  // } Stats;

  struct {
    // New abstraction: Groups is used in stead of Tubes(LOKI), Triplets(BIFROST),
    // TubePair(MIRACLES) etc.
    int Groups{0};
    // New abstraction: GroupSize is used instead of Straws(LOKI), Tubes(BIFROST)
    int GroupSize{0};
    int Amplitudes{2};
    int Pixellation{0};
    std::vector<float> DefaultIntervals;

  } Parms;


  // File and Json
  std::string ConfigFile{""};
  nlohmann::json root;

private:

  void consistencyCheck(); // called from parseCaibration()
  nlohmann::json getObjectAndCheck(nlohmann::json JsonObject, std::string Property);

  std::string Name;
  uint32_t MaxPixelId{0};      ///< The maximum pixelid in the map
};
} // namespace Caen
