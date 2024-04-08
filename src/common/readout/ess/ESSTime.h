// Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS HighTime/LowTime handler class
///
//===----------------------------------------------------------------------===//

#pragma once

#include "common/time/TimeNano.h"
#include <cassert>
#include <chrono>
#include <cinttypes>
#include <common/debug/Trace.h>
#include <cstdint>
#include <gtest/internal/gtest-internal.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace ESSReadout {

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
  uint64_t setReference(const ESSTime &refESSTime) {
    TimeInNS = refESSTime.toNS();
    return TimeInNS.count();
  }

  uint64_t setPrevReference(const ESSTime &refPrevESSTime) {
    PrevTimeInNS = refPrevESSTime.toNS();
    return PrevTimeInNS.count();
  }

  inline ESSTime getRefESSTime() const { return ESSTime(TimeInNS); }

  inline ESSTime getPrevRefESSTime() const { return ESSTime(PrevTimeInNS); }

  inline TimeDurationNano getRefTimeNS() const { return TimeInNS; }

  inline TimeDurationNano getPrevRefTimeNS() const { return PrevTimeInNS; }

  inline uint64_t getRefTimeUInt64() const { return TimeInNS.count(); }

  inline uint64_t getPrevRefTimeUInt64() const { return PrevTimeInNS.count(); }

  void setMaxTOF(uint64_t NewMaxTOF) { MaxTOF = TimeDurationNano(NewMaxTOF); }
  /// \brief calculate TOF from saved reference and current event time
  uint64_t getTOF(ESSTime eventEssTime, uint32_t DelayNS = 0) {
    TimeDurationNano timeval = eventEssTime.toNS() + TimeDurationNano(DelayNS);
    if (timeval < TimeInNS) {
      XTRACE(EVENT, WAR,
             "TOF negative: High: 0x%08x, Low: 0x%08x, timens %" PRIu64,
             ", PrevPTns: %" PRIu64, eventEssTime.getTimeHigh(),
             eventEssTime.getTimeLow(), timeval, TimeInNS);
      Stats.TofNegative++;
      return getPrevTOF(eventEssTime, DelayNS);
    }
    if ((timeval - TimeInNS) > MaxTOF) {
      XTRACE(EVENT, WAR, "High TOF: High: 0x%08x, Low: 0x%08x, timens %" PRIu64,
             ", PrevPTns: %" PRIu64, eventEssTime.getTimeHigh(),
             eventEssTime.getTimeLow(), timeval, TimeInNS);
      Stats.TofHigh++;
      return InvalidTOF;
    }
    Stats.TofCount++;
    return (timeval - TimeInNS).count();
  }

  /// \brief calculate TOF from saved reference and current event time
  /// \todo a valid value of TOF = 0 is in
  uint64_t getPrevTOF(ESSTime eventEssTime, uint32_t DelayNS = 0) {
    TimeDurationNano timeval = eventEssTime.toNS() + TimeDurationNano(DelayNS);
    if (timeval < PrevTimeInNS) {
      XTRACE(EVENT, WAR,
             "Prev TOF negative: High: 0x%04x, Low: 0x%04x, timens %" PRIu64,
             ", PrevPTns: %" PRIu64, eventEssTime.getTimeHigh(),
             eventEssTime.getTimeLow(), timeval, PrevTimeInNS);
      Stats.PrevTofNegative++;
      return InvalidTOF;
    }
    if ((timeval - PrevTimeInNS) > MaxTOF) {
      XTRACE(EVENT, WAR,
             "High Prev TOF: High: 0x%04x, Low: 0x%04x, timens %" PRIu64,
             ", PrevPTns: %" PRIu64, eventEssTime.getTimeHigh(),
             eventEssTime.getTimeLow(), timeval, PrevTimeInNS);
      Stats.PrevTofHigh++;
      return InvalidTOF;
    }
    Stats.PrevTofCount++;
    return (timeval - PrevTimeInNS).count();
  }

  struct Stats_t Stats = {};

private:
  TimeDurationNano TimeInNS{0};
  TimeDurationNano PrevTimeInNS{0};
  TimeDurationNano MaxTOF{
      2147483647}; // max 32 bit integer, larger TOFs cause errors downstream
};
} // namespace ESSReadout
