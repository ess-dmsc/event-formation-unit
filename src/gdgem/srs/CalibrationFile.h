/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for handling calibration files for VMM asics
///
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <vector>

namespace Gem {

/// \todo check whether packing is necessary, static assert assert?
struct Calibration {
  //Apple clang demands constructors for the struct
  Calibration()= default;
  Calibration(float adcOffset,float adcSlope,float timeOffset,float timeSlope):
  	adc_offset(adcOffset), adc_slope(adcSlope), time_offset(timeOffset), time_slope(timeSlope) 
  {}

  float adc_offset {0.0};
  float adc_slope {1.0};
  float time_offset {0.0};
  float time_slope {1.0};
};

class CalibrationFile {
public:
    static constexpr size_t MAX_FEC {40};
    static constexpr size_t MAX_VMM {16};
    static constexpr size_t MAX_CH  {64};

  /// \brief create default calibration (0.0 offset 1.0 slope)
  CalibrationFile() = default;
  
  /// \brief load calibration from json file
  explicit CalibrationFile(std::string filename);

  /// \brief loads calibration from json string
  void loadCalibration(std::string calibration);

  /// \brief Generate fast mappings from IDs to indexes
  void addCalibration(size_t fecId, size_t vmmId, size_t chNo,
                      float adc_offset, float adc_slope, 
                      float time_offset, float time_slope);

  /// \brief get calibration data for (fec, vmm, channel)
  /// \todo check how vmm3 data is supplied, maybe getting an array for a given
  /// (fec, vmm) is better?
  const Calibration& getCalibration(size_t fecId, size_t vmmId, size_t chNo) const;

  std::string debug() const;

private:
  std::vector<std::vector<std::vector<Calibration>>> Calibrations;

  /// Default correction
  Calibration NoCorr {0.0, 1.0, 0.0, 1.0};
  
};

}
