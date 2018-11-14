/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple struct for containing pulse data.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcTimeStamp.h"
#include "ChannelID.h"
#include "SamplingRun.h"

/// \brief Stores data about a pulse from the ADC system, including the source.
struct PulseParameters {
  ChannelID Identifier{0, 0};
  /// Background value at location of peak
  int BackgroundLevel{0};
  /// Maximum value of peak
  int PeakLevel{0};
  /// Difference between peak and background at peak
  int PeakAmplitude{0};
  /// Area under the curve with background subtracted
  int PeakArea{0};
  /// Timestamp of maximum of peak
  RawTimeStamp PeakTimestamp;
  /// Timestamp where line goes above threshold
  RawTimeStamp ThresholdTimestamp;
};

/// \brief Implements the data extraction information as  pure function.
///
/// \param[in] SampleRun A vector of samples to find the peak in.
/// \param[in] Threshold
/// \return The results of the peak finding algorithm.
PulseParameters analyseSampleRun(SamplingRun const &Run, double Threshold);
