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

  VMM3Calibration() {};

  VMM3Calibration(double TOffset, double TSlope, double AOffset, double ASlope) :
    TDCOffset(TOffset), TDCSlope(TSlope),
    ADCOffset(AOffset), ADCSlope(ASlope) {}

    double TDCCorr(uint8_t TDC) {
      double TDCns = 1.5 * 22.72 - 60.0*TDC/255;
      double TDCCorr = (TDCns - TDCOffset) * TDCSlope;
      return TDCCorr;
    }

    double ADCCorr(uint16_t ADC) {
      double ADCCorr = (ADC - ADCOffset) * ADCSlope;
      return std::max(std::min(1023.0, ADCCorr), 0.0);
    }

private:
  double TDCOffset{0.0};
  double TDCSlope{1.0};
  double ADCOffset{0.0};
  double ADCSlope{1.0};
};
