/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief For doing time stamp calculations.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcReadoutConstants.h"

#include <cmath>
#include <cstdint>
#include <chrono>
#include <netinet/in.h>

const static std::uint64_t NSecMultiplier = 1'000'000'000;

struct Timing {
  constexpr Timing(std::uint64_t const ClockFrequency)
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

struct RawTimeStamp {
  /// \brief Number of seconds since UNIX epoch (1980-01-01 00:00).
  /// This value is provided originally by the MRF hardware.
  /// \note The epoch might change in the future.
  std::uint32_t Seconds{0};

  /// \brief Number of clock cycles of the timing hardware clock divided by two.
  // This value resets to zero once every second and has a maximum value of ≈44
  // million.
  // \note It is possible that in future systems, the clock frequency will not
  // be 44 MHz.
  std::uint32_t SecondsFrac{0};

  void fixEndian() {
    Seconds = ntohl(Seconds);
    SecondsFrac = ntohl(SecondsFrac);
  }
} __attribute__((packed));

/// \brief Maps to the timestamps as received from (and used by) the ADC
/// hardware.
class TimeStamp {
public:
  enum class ClockMode { External = 1, Internal = 0 };
  TimeStamp() = default;

  /// \brief Create a raw timestamp from a timestamp in nanoseconds since Unix
  /// epoch.
  TimeStamp(std::uint64_t NSec, ClockMode Mode);

  /// \brief Create a raw timestamp from a timing system timestamp.
  TimeStamp(RawTimeStamp Time, ClockMode Mode) : CTime(Time), CMode(Mode) {}

  bool operator==(TimeStamp const &Other) const;

  /// \brief Calculate the raw timestamp based on sample offset.
  /// Adds or subtracts the specified number of clock cycles (samples).
  /// Correctly wraps around the fractional seconds if so required to get the
  /// correct value.
  /// \param[in] SampleOffset The number of samples that the time stamp should
  /// be offset. Can be negative and positive.
  /// \return The resulting new timestamp.
  TimeStamp getOffsetTimeStamp(const std::int32_t &SampleOffset) const;

  /// \brief Convert from raw timestamp (seconds plus fractional seconds) into
  /// nanoseconds since epoch.
  /// \return Number of nanoseconds since epoch (currently EPICS epoch).
  std::uint64_t getTimeStampNS() const;

  ClockMode getClockMode() const { return CMode; }

  double getClockCycleLength() const;

  std::uint32_t getSeconds() const { return CTime.Seconds; }

  std::uint32_t getSecondsFrac() const { return CTime.SecondsFrac; }

  void reset() {
    CTime = {0, 0};
    CMode = ClockMode::External;
  }

private:
  RawTimeStamp CTime{0, 0};
  ClockMode CMode{ClockMode::External};
};

template <typename ChronoTime>
TimeStamp MakeTimeStampFromClock(ChronoTime TimeNow) {
  auto NowSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                        TimeNow.time_since_epoch())
                        .count();
  double NowSecFrac = (std::chrono::duration_cast<std::chrono::nanoseconds>(
                           TimeNow.time_since_epoch())
                           .count() /
                       1e9) -
                      NowSeconds;
  std::uint32_t Ticks = std::lround(NowSecFrac * (88052500 / 2.0));

  RawTimeStamp rts{static_cast<uint32_t>(NowSeconds), Ticks};
  return TimeStamp(rts, TimeStamp::ClockMode::External);
}

//-----------------------------------------------------------------------------

inline TimeStamp::TimeStamp(std::uint64_t NSec, ClockMode Mode) : CMode(Mode) {
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

inline std::uint64_t TimeStamp::getTimeStampNS() const {
  auto NanoSec = static_cast<std::uint64_t>(std::llround(
      static_cast<double>(CTime.SecondsFrac) /
      static_cast<double>(TimeConst[int(CMode)].AdcTimerCounterMax) * 1e9));
  return static_cast<std::uint64_t>(
      static_cast<std::uint64_t>(CTime.Seconds) * NSecMultiplier + NanoSec);
}

inline TimeStamp
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

inline bool TimeStamp::operator==(const TimeStamp &Other) const {
  return Other.CTime.Seconds == CTime.Seconds and
         Other.CTime.SecondsFrac == CTime.SecondsFrac and Other.CMode == CMode;
}

inline double TimeStamp::getClockCycleLength() const {
  return 1e9 / TimeConst[int(CMode)].AdcTimerCounterMax;
}
