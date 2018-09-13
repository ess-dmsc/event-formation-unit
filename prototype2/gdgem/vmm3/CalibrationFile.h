/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for handling calibraiton files for VMM asics
///
//===----------------------------------------------------------------------===//

#include <string>

class CalibrationFile {
public:
  static constexpr int MAX_FEC = 40;
  static constexpr int MAX_VMM = 16;
  static constexpr int MAX_CH   = 64;

  /// \todo check whether packing is necessary, static assert assert?
  struct calibration_t {
    float offset;
    float slope;
  };

  ///
  CalibrationFile();

  /// \brief loads calibration from json string
  void LoadCalibration(std::string calibration);

  /// \brief Generate fast mappings from IDs to indexes
  int addCalibration(unsigned int fecId, unsigned int vmmId, unsigned int chNo, float offset, float slope);

  /// \brief get calibration data for (fec, vmm, channel)
  /// \todo check how vmm3 data is supplied, maybe getting an array for a given (fec, vmm) is better?
  struct calibration_t & getCalibration(unsigned int fecId, unsigned int vmmId, unsigned int chNo);

private:

  struct calibration_t calibrations [MAX_FEC][MAX_VMM][MAX_CH];

  /// Default correction
  struct calibration_t nocorr = {0.0, 1.0};

  /// Slope zero indicates an error
  struct calibration_t errcorr = {0.0, 0.0};
};
