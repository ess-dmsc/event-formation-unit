/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief
///
//===----------------------------------------------------------------------===//

#include "PulseParameters.h"
#include <cmath>
#include <numeric>

static const int BeginBkgSamples = 1;
static const int EndBkgSamples = 1;

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
  auto BkgSlope = (EndBkgMean - BeginBkgMean) / double(Run.Data.size());
  auto PeakPosition = 0u;
  for (auto i = 0u; i < Run.Data.size(); i++) {
    auto TempBackground = BkgOffsetValue + std::round(i * BkgSlope);
    Result.PeakArea += (Run.Data[i] - TempBackground);
    if (Run.Data[i] - TempBackground > Result.PeakLevel) {
      PeakPosition = i;
      Result.PeakLevel = Run.Data[i];
      Result.PeakTimestamp = Run.TimeStamp.GetOffsetTimeStamp(i);
      Result.BackgroundLevel = TempBackground;
      Result.PeakAmplitude = Result.PeakLevel - Result.BackgroundLevel;
    }
  }
  for (auto i = 0u; i <= PeakPosition; ++i) {
    auto TempBackground = BkgOffsetValue + std::round(i * BkgSlope);
    auto CurrentPeakLevel = Run.Data[i] - TempBackground;
    if (CurrentPeakLevel / Result.PeakAmplitude > ThresholdLevel) {
      Result.ThresholdTimestamp = Run.TimeStamp.GetOffsetTimeStamp(i);
      break;
    }
  }
  return Result;
}
