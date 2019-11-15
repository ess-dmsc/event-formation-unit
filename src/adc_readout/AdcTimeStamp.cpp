/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief For doing time stamp calculations.
 */

#include "AdcTimeStamp.h"
#include "AdcReadoutConstants.h"
#include <cmath>

const static std::uint64_t NSecMultiplier = 1000000000;

struct Timing {
  constexpr Timing(std::uint64_t ClockFrequency)
      : TimeClockFrequency(ClockFrequency), SamplingRate(ClockFrequency / 2),
        AdcTimerCounterMax(SamplingRate), SampleLengthNS(2e9 / ClockFrequency) {
  }
  std::uint32_t const TimeClockFrequency;
  std::uint32_t const SamplingRate;
  std::uint32_t const AdcTimerCounterMax;
  double const SampleLengthNS;
};

static constexpr Timing const TimeConst[2] = {TimerClockFrequencyInternal,
                                              TimerClockFrequencyExternal};

TimeStamp::TimeStamp(std::uint64_t NSec, ClockMode Mode) : CMode(Mode) {
  CTime.Seconds = NSec / NSecMultiplier;
  auto NanoSecPart = NSec % NSecMultiplier;
  auto &CConst = TimeConst[int(CMode)];
  CTime.SecondsFrac =
      std::lround(NanoSecPart / TimeConst[int(CMode)].SampleLengthNS);
  if (CTime.SecondsFrac == CConst.AdcTimerCounterMax) {
    ++CTime.Seconds;
    CTime.SecondsFrac = 0;
  }
}

std::uint64_t TimeStamp::getTimeStampNS() const {
  auto NanoSec = static_cast<std::uint64_t>(std::llround(
      static_cast<double>(CTime.SecondsFrac) /
      static_cast<double>(TimeConst[int(CMode)].AdcTimerCounterMax) * 1e9));
  return static_cast<std::uint64_t>(
      static_cast<std::uint64_t>(CTime.Seconds) * NSecMultiplier + NanoSec);
}

// Note: This function might be significantly slower than CalcTimeStamp() for
// some cases.
std::uint64_t TimeStamp::getTimeStampNSFast() const {

  std::uint64_t NanoSec = (((CTime.SecondsFrac * 100000000000) /
                            (TimeConst[int(CMode)].AdcTimerCounterMax)) +
                           50) /
                          100;
  return static_cast<std::uint64_t>(
      static_cast<std::uint64_t>(CTime.Seconds) * 100000000000 + NanoSec);
}

TimeStamp
TimeStamp::getOffsetTimeStamp(const std::int32_t &SampleOffset) const {
  std::int32_t TempSecondsFrac = CTime.SecondsFrac + SampleOffset;
  std::int32_t RemainderSecondsFrac =
      TempSecondsFrac %
      static_cast<std::int32_t>(TimeConst[int(CMode)].AdcTimerCounterMax);
  std::int32_t NewSecondsFrac = RemainderSecondsFrac;
  int SecondsChange =
      static_cast<std::int32_t>(TempSecondsFrac) /
      static_cast<std::int32_t>(TimeConst[int(CMode)].AdcTimerCounterMax);
  if (TempSecondsFrac < 0) {
    SecondsChange = -SecondsChange - 1;
  }
  if (RemainderSecondsFrac < 0) {
    NewSecondsFrac =
        TimeConst[int(CMode)].AdcTimerCounterMax + RemainderSecondsFrac;
  }
  return {RawTimeStamp{CTime.Seconds + SecondsChange,
                       static_cast<uint32_t>(NewSecondsFrac)},
          CMode};
}

bool TimeStamp::operator==(const TimeStamp &Other) const {
  return Other.CTime.Seconds == CTime.Seconds and
         Other.CTime.SecondsFrac == CTime.SecondsFrac and Other.CMode == CMode;
}

double TimeStamp::getClockCycleLength() const {
  return 1e9 / TimeConst[int(CMode)].AdcTimerCounterMax;
}
