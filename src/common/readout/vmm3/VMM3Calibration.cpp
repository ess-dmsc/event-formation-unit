// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief VMM3Calibration class
///
//===----------------------------------------------------------------------===//


#include <common/readout/vmm3/VMM3Calibration.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

bool VMM3Calibration::setCalibration(int Channel,
                 double TDCOffset, double TDCSlope,
                 double ADCOffset, double ADCSlope) {
    if (Channel >= CHANNELS) {
      return false;
    }
    Calibration[Channel].TDCOffset = TDCOffset;
    Calibration[Channel].TDCSlope = TDCSlope;
    Calibration[Channel].ADCOffset = ADCOffset;
    Calibration[Channel].ADCSlope = ADCSlope;
    return true;
}

double VMM3Calibration::TDCCorr(int Channel, uint8_t TDC) {
	double TDCns = 1.5 * 22.72 - 60.0*TDC/255;
	double TDCCorr = (TDCns - Calibration[Channel].TDCOffset)
	               * Calibration[Channel].TDCSlope;
	return TDCCorr;
}

double VMM3Calibration::ADCCorr(int Channel, uint16_t ADC) {
	double ADCCorr = (ADC - Calibration[Channel].ADCOffset)
	               * Calibration[Channel].ADCSlope;
	return std::max(std::min(1023.0, ADCCorr), 0.0);
}