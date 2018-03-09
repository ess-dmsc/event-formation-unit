/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief For doing time stamp calculations.
 */

#include "AdcTimeStamp.h"
#include <cmath>

static const std::int32_t TimerCounterMax = 88052500 / 2;

std::uint64_t RawTimeStamp::GetTimeStampNS() const {
  auto NanoSec = static_cast<std::uint64_t>(
      std::llround(static_cast<double>(SecondsFrac) /
                   static_cast<double>(TimerCounterMax) * 1e9));
  return static_cast<std::uint64_t>(
      static_cast<std::uint64_t>(Seconds) * 1000000000 + NanoSec);
}

// Note: This function might be significantly slower than CalcTimeStamp() for
// some cases.
std::uint64_t RawTimeStamp::GetTimeStampNSFast() const {
  const std::uint64_t Multiplier = 100000000000;
  std::uint64_t NanoSec =
      (((SecondsFrac * Multiplier) / (TimerCounterMax)) + 50) / 100;
  return static_cast<std::uint64_t>(
      static_cast<std::uint64_t>(Seconds) * 1000000000 + NanoSec);
}

RawTimeStamp
RawTimeStamp::GetOffsetTimeStamp(const std::int32_t &SampleOffset) const {
  std::int32_t TempSecondsFrac = SecondsFrac + SampleOffset;
  std::int32_t RemainderSecondsFrac = TempSecondsFrac % TimerCounterMax;
  std::int32_t NewSecondsFrac = RemainderSecondsFrac;
  int SecondsChange =
      static_cast<std::int32_t>(TempSecondsFrac) / TimerCounterMax;
  if (TempSecondsFrac < 0) {
    SecondsChange = -SecondsChange - 1;
  }
  if (RemainderSecondsFrac < 0) {
    NewSecondsFrac = TimerCounterMax + RemainderSecondsFrac;
  }
  return {Seconds + SecondsChange, static_cast<uint32_t>(NewSecondsFrac)};
}
