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

  void setCalibration(int Channel,
                 double TDCOffset, double TDCSlope,
                 double ADCOffset, double ADCSlope) {
    if (Channel >= CHANNELS) {
      // XTRACE()
      return;
    }

    Calibration[Channel].TDCOffset = TDCOffset;
    Calibration[Channel].TDCSlope = TDCSlope;
    Calibration[Channel].ADCOffset = ADCOffset;
    Calibration[Channel].ADCSlope = ADCSlope;
  }

  double TDCCorr(int Channel, uint8_t TDC) {
    double TDCns = 1.5 * 22.72 - 60.0*TDC/255;
    double TDCCorr = (TDCns - Calibration[Channel].TDCOffset)
                   * Calibration[Channel].TDCSlope;
    return TDCCorr;
  }

  double ADCCorr(int Channel, uint16_t ADC) {
    double ADCCorr = (ADC - Calibration[Channel].ADCOffset)
                   * Calibration[Channel].ADCSlope;
    return std::max(std::min(1023.0, ADCCorr), 0.0);
  }

private:

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
