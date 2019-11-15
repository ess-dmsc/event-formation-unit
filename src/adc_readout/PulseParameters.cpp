/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief
///
//===----------------------------------------------------------------------===//

#include "PulseParameters.h"
#include "AdcReadoutConstants.h"
#include <cmath>
#include <numeric>

static const int BeginBkgSamples = 1;
static const int EndBkgSamples = 1;

float interpolateThreshold(double ValueA, double ValueB, double Threshold) {
  return (Threshold - ValueA) / (ValueB - ValueA);
}

PulseParameters analyseSampleRun(SamplingRun const &Run,
                                 double ThresholdLevel) {
  PulseParameters Result;

  Result.Identifier = Run.Identifier;

  auto BeginBkgMean =
      std::accumulate(Run.Data.begin(), Run.Data.begin() + BeginBkgSamples,
                      0.0f) /
      BeginBkgSamples;

  auto EndBkgMean =
      std::accumulate(Run.Data.end() - EndBkgSamples, Run.Data.end(), 0.0f) /
      EndBkgSamples;

  auto BkgOffsetValue = Run.Data[0];
  auto BkgSlope = (EndBkgMean - BeginBkgMean) / double(Run.Data.size() - 1);
  auto PeakPosition = 0u;
  for (auto i = 0u; i < Run.Data.size(); i++) {
    auto TempBackground = BkgOffsetValue + std::round(i * BkgSlope);
    Result.PeakArea += std::lround(Run.Data[i] - TempBackground);
    if (Run.Data[i] - TempBackground > Result.PeakAmplitude) {
      PeakPosition = i;
      Result.PeakLevel = Run.Data[i];
      Result.PeakTime = Run.StartTime.getOffsetTimeStamp(i);
      Result.BackgroundLevel = std::lround(TempBackground);
      Result.PeakAmplitude = Result.PeakLevel - Result.BackgroundLevel;
    }
  }
  if (Run.Data.size() == 1 or PeakPosition < 1) {
    Result.ThresholdTime = Run.StartTime;
    Result.ThresholdTimestampNS = Run.StartTime.getTimeStampNS();
    return Result;
  }
  for (auto i = 1u; i <= PeakPosition; ++i) {
    auto CurBkg = BkgOffsetValue + std::round(i * BkgSlope);
    auto CurAmp = (Run.Data[i] - CurBkg) / Result.PeakAmplitude;
    if (CurAmp >= ThresholdLevel) {
      auto PrevBkg = BkgOffsetValue + std::round((i - 1) * BkgSlope);
      auto PrevAmp = (Run.Data[i - 1] - PrevBkg) / Result.PeakAmplitude;
      auto ThresholdSubPosition =
          interpolateThreshold(PrevAmp, CurAmp, ThresholdLevel);
      auto ThresholdPosition = (i - 1) + ThresholdSubPosition;
      auto ActualThresholdPosition = ThresholdPosition * Run.OversamplingFactor;
      Result.ThresholdTime =
          Run.StartTime.getOffsetTimeStamp(lround(ActualThresholdPosition));
      auto TempNSTimestampCalc =
          Run.StartTime.getOffsetTimeStamp(int(ActualThresholdPosition));
      Result.ThresholdTimestampNS =
          TempNSTimestampCalc.getTimeStampNS() +
          llround((ActualThresholdPosition - int(ActualThresholdPosition)) *
                  Run.StartTime.getClockCycleLength());
      break;
    }
  }
  return Result;
}
