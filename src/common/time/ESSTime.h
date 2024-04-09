// Copyright (C) 2019-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ESS time related methoods and classes
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <chrono>
#include <cinttypes>
#include <common/debug/Trace.h>
#include <cstdint>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace esstime {

using TimePointNano = std::chrono::high_resolution_clock::time_point;
using TimeDurationNano = std::chrono::duration<size_t, std::nano>;

class ESSTime {

  uint32_t TimeHigh;
  uint32_t TimeLow;

public:
  static constexpr TimeDurationNano SecInNs =
      std::chrono::duration_cast<TimeDurationNano>(std::chrono::seconds(1));
  static constexpr double ESSClockFreqHz{88052500};
  static constexpr double ESSClockTick{SecInNs.count() / ESSClockFreqHz};

  ESSTime() : TimeHigh(0), TimeLow(0) {}

  ESSTime(const uint32_t High, const uint32_t Low)
      : TimeHigh(High), TimeLow(Low) {}

  ESSTime(const TimeDurationNano timeInNs)
      : TimeHigh(
            std::chrono::duration_cast<std::chrono::seconds>(timeInNs).count()),
        TimeLow(round((timeInNs.count() - (TimeHigh * SecInNs.count())) /
                      ESSClockTick)) {}

  ESSTime(const ESSTime &other)
      : TimeHigh(other.TimeHigh), TimeLow(other.TimeLow) {}

  inline void operator+=(const TimeDurationNano &nanoseconds) {
    uint32_t ticksToAdd = nanoseconds.count() / ESSClockTick;
    this->operator+=(ticksToAdd);
  }

  inline void operator+=(uint32_t &ticks) {
    uint32_t newTimeLow = TimeLow + ticks;

    if (newTimeLow > ESSClockFreqHz) {
      newTimeLow -= ESSClockFreqHz;
      TimeHigh += 1;
    }

    TimeLow = newTimeLow;
  }

  ESSTime &operator=(const ESSTime &other) {
    if (this != &other) {
      TimeHigh = other.TimeHigh;
      TimeLow = other.TimeLow;
    }

    return *this;
  }

  inline TimeDurationNano toNS() const { return toNS(TimeHigh, TimeLow); }

  /// \brief convert ess High/Low time to NS
  static TimeDurationNano toNS(const uint32_t &High, const uint32_t &Low) {
    return TimeDurationNano(High * SecInNs.count() +
                            (uint64_t)(Low * ESSClockTick));
  }

  inline uint32_t getTimeHigh() const { return TimeHigh; }

  inline uint32_t getTimeLow() const { return TimeLow; }
};

class ESSReferenceTime {

public:
  struct Stats_t {
    int64_t TofCount;
    int64_t TofNegative;
    int64_t PrevTofCount;
    int64_t PrevTofNegative;
    int64_t TofHigh;
    int64_t PrevTofHigh;
  };

  ESSReferenceTime() = default;

  ESSReferenceTime(ESSTime pulseTime) : TimeInNS(pulseTime.toNS()){};

  const uint64_t InvalidTOF{0xFFFFFFFFFFFFFFFFULL};

  /// \brief save reference (pulse) time
  uint64_t setReference(const ESSTime &refESSTime);

  uint64_t setPrevReference(const ESSTime &refPrevESSTime);

  void setMaxTOF(uint64_t NewMaxTOF);

  inline ESSTime getRefESSTime() const { return ESSTime(TimeInNS); }

  inline ESSTime getPrevRefESSTime() const { return ESSTime(PrevTimeInNS); }

  inline TimeDurationNano getRefTimeNS() const { return TimeInNS; }

  inline TimeDurationNano getPrevRefTimeNS() const { return PrevTimeInNS; }

  inline uint64_t getRefTimeUInt64() const { return TimeInNS.count(); }

  inline uint64_t getPrevRefTimeUInt64() const { return PrevTimeInNS.count(); }

  /// \brief calculate TOF from saved reference and current event time
  uint64_t getTOF(ESSTime eventEssTime, uint32_t DelayNS = 0);

  /// \brief calculate TOF from saved reference and current event time
  /// \todo a valid value of TOF = 0 is in
  uint64_t getPrevTOF(ESSTime eventEssTime, uint32_t DelayNS = 0);

  struct Stats_t Stats = {};

private:
  TimeDurationNano TimeInNS{0};
  TimeDurationNano PrevTimeInNS{0};
  TimeDurationNano MaxTOF{
      2147483647}; // max 32 bit integer, larger TOFs cause errors downstream
};

} // namespace esstime
