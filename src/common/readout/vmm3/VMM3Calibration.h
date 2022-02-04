// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief VMM3Calibration class
///
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <cinttypes>
#include <common/debug/Trace.h>


// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class VMM3Calibration {
public:
  static constexpr int CHANNELS{64};

  struct Calib {
    double TDCOffset;
    double TDCSlope;
    double ADCOffset;
    double ADCSlope;
  };

  VMM3Calibration() {
    InitCal();
  };

  ///\brief Set the calibration parameters for the specified channel
  ///\param Channel VMM Channel
  ///\param TDCOffset (see VMM3 ICD)
  ///\param TDCSlope (see VMM3 ICD)
  ///\param ADCOffset (see VMM3 ICD)
  ///\param ADCSlope (see VMM3 ICD)
  ///\returns true if channel is valid (configuration was set) else false
  bool setCalibration(int Channel,
                 double TDCOffset, double TDCSlope,
                 double ADCOffset, double ADCSlope);

  ///\brief return the corrected TDC time in ns for the specified channel
  /// It is assumed that Channel is within the valid range (0 - 63)
  double TDCCorr(int Channel, uint8_t TDC);

  ///\brief return the corrected ADC value for the specified channel
  /// Values are clamped to 0 or 1023 if correction falls outside
  /// the valid ranges.
  /// It is assumed that Channel is within the valid range (0 - 63)
  double ADCCorr(int Channel, uint16_t ADC);

private:
  ///\brief the initial calibration is the identity calibration with
  /// offsets 0.0 and slopes 1.0
  void InitCal() {
    for (auto & Cal : Calibration) {
      Cal.TDCOffset = 0.0;
      Cal.TDCSlope = 1.0;
      Cal.ADCOffset = 0.0;
      Cal.ADCSlope = 1.0;
    }
  }

  struct Calib Calibration[CHANNELS];
};
