/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple peak finding implementation.
 */

#include "DelayLineProcessing.h"
#include <cmath>
#include <limits>

DelayLineProcessing::DelayLineProcessing(
    std::shared_ptr<DelayLineProducer> Prod)
    : AdcDataProcessor(std::move(Prod)) {}

void DelayLineProcessing::processData(SamplingRun const &Data) {
  dynamic_cast<DelayLineProducer *>(ProducerPtr.get())
      ->addPulse(analyseSampleRun(Data));
}

static const int BeginBkgSamples = 1;
static const int EndBkgSamples = 1;
static const int ThresholdLevel = 100;

PulseParameters analyseSampleRun(SamplingRun const &Run) {
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

  auto PastThreshold = false;
  for (auto i = 0u; i < Run.Data.size(); i++) {
    auto TempBackground = BkgOffsetValue + std::round(i * BkgSlope);
    Result.PeakArea += (Run.Data[i] - TempBackground);
    if (Run.Data[i] > Result.PeakLevel) {
      Result.PeakLevel = Run.Data[i];
      Result.PeakTimestamp = Run.TimeStamp.GetOffsetTimeStamp(i);
      Result.BackgroundLevel = TempBackground;
      Result.PeakAmplitude = Result.PeakLevel - Result.BackgroundLevel;
    }
    if (not PastThreshold and Run.Data[i] > TempBackground + ThresholdLevel) {
      PastThreshold = true;
      Result.ThresholdTimestamp = Run.TimeStamp.GetOffsetTimeStamp(i);
      /// \todo Improve the threshold timestamp calculation algorithm.
    }
  }
  return Result;
}
