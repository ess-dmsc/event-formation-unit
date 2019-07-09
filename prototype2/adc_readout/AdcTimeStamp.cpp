/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief For doing time stamp calculations.
 */

#include "AdcTimeStamp.h"
#include "AdcReadoutConstants.h"
#include <cmath>

const static std::uint64_t NSecMultiplier = 1000000000;

RawTimeStamp::RawTimeStamp(std::uint64_t NSec) : Seconds(NSec / NSecMultiplier) {
  auto NanoSecPart = NSec % NSecMultiplier;
  SecondsFrac = std::lround(NanoSecPart / SampleLengthNS);
  if (SecondsFrac == AdcTimerCounterMax) {
    ++Seconds;
    SecondsFrac = 0;
  }
}

std::uint64_t RawTimeStamp::GetTimeStampNS() const {
  auto NanoSec = static_cast<std::uint64_t>(
      std::llround(static_cast<double>(SecondsFrac) /
                   static_cast<double>(AdcTimerCounterMax) * 1e9));
  return static_cast<std::uint64_t>(
      static_cast<std::uint64_t>(Seconds) * NSecMultiplier + NanoSec);
}

// Note: This function might be significantly slower than CalcTimeStamp() for
// some cases.
std::uint64_t RawTimeStamp::GetTimeStampNSFast() const {

  std::uint64_t NanoSec =
      (((SecondsFrac * 100000000000) / (AdcTimerCounterMax)) + 50) / 100;
  return static_cast<std::uint64_t>(
      static_cast<std::uint64_t>(Seconds) * 100000000000 + NanoSec);
}

RawTimeStamp
RawTimeStamp::GetOffsetTimeStamp(const std::int32_t &SampleOffset) const {
  std::int32_t TempSecondsFrac = SecondsFrac + SampleOffset;
  std::int32_t RemainderSecondsFrac = TempSecondsFrac % AdcTimerCounterMax;
  std::int32_t NewSecondsFrac = RemainderSecondsFrac;
  int SecondsChange =
      static_cast<std::int32_t>(TempSecondsFrac) / AdcTimerCounterMax;
  if (TempSecondsFrac < 0) {
    SecondsChange = -SecondsChange - 1;
  }
  if (RemainderSecondsFrac < 0) {
    NewSecondsFrac = AdcTimerCounterMax + RemainderSecondsFrac;
  }
  return {Seconds + SecondsChange, static_cast<uint32_t>(NewSecondsFrac)};
}

bool RawTimeStamp::operator==(const RawTimeStamp &Other) const {
  return Other.Seconds == Seconds and Other.SecondsFrac == SecondsFrac;
}
