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

namespace Gem {

class CalibrationFile {
public:
  static constexpr int MAX_FEC = 40;
  static constexpr int MAX_VMM = 16;
  static constexpr int MAX_CH = 64;

  /// \todo check whether packing is necessary, static assert assert?
  typedef struct {
    float offset;
    float slope;
  } Calibration;

  /// \brief create default calibration (0.0 offset 1.0 slope)
  CalibrationFile();

  /// \brief load calibration from json file
  CalibrationFile(std::string filename);

  /// \brief loads calibration from json string
  void loadCalibration(std::string calibration);

  /// \brief Generate fast mappings from IDs to indexes
  bool addCalibration(unsigned int fecId, unsigned int vmmId, unsigned int chNo,
                      float offset, float slope);

  /// \brief get calibration data for (fec, vmm, channel)
  /// \todo check how vmm3 data is supplied, maybe getting an array for a given
  /// (fec, vmm) is better?
  Calibration &getCalibration(unsigned int fecId, unsigned int vmmId,
                              unsigned int chNo);

private:
  void resetCalibration();

  Calibration Calibrations[MAX_FEC][MAX_VMM][MAX_CH];

  /// Default correction
  Calibration NoCorr = {0.0, 1.0};

  /// Slope zero indicates an error
  Calibration ErrCorr = {0.0, 0.0};
};

}
