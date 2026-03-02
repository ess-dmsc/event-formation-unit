// Copyright (C) 2019 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ESS time related methods and classes
///
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <climits>
#include <cmath>
#include <common/StatCounterBase.h>
#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>

namespace esstime {

using local_clock = std::chrono::high_resolution_clock;
using tof_t = std::optional<uint64_t>;

constexpr std::nullopt_t InvalidTof = std::nullopt;

using TimePointNano = std::chrono::high_resolution_clock::time_point;
/// \brief Duration in nanoseconds with int64_t precision
using TimeDurationNano = std::chrono::duration<int64_t, std::nano>;
/// \brief Duration in microseconds with uint64_t precision
using TimeDurationMicro = std::chrono::duration<double, std::micro>;
/// \brief Duration in milliseconds with double precision
using TimeDurationMilli = std::chrono::duration<double, std::milli>;
/// \brief Duration in seconds with double precision
using TimeDurationSec = std::chrono::duration<double, std::ratio<1>>;

/// \brief Converts a frequency in Hertz to a period in nanoseconds.
/// \param FrequencyHz Frequency in Hertz.
/// \return std::chrono::nanoseconds The corresponding period in nanoseconds.
inline std::chrono::nanoseconds hzToNanoseconds(uint32_t FrequencyHz) {
  if (FrequencyHz == 0) {
    throw std::invalid_argument(
        "Frequency cannot be zero, division by zero is not allowed.");
  }
  uint64_t nsPeriod = static_cast<uint64_t>(1e9 / FrequencyHz);
  return TimeDurationNano(nsPeriod);
}

/// \brief Converts a duration in nanoseconds (int64_t) to microseconds.
/// \param NanoSeconds Duration in nanoseconds.
/// \return TimeDurationMicro(double) The corresponding duration in
/// microseconds.
inline TimeDurationMicro nsToMicroseconds(int64_t NanoSeconds) {
  return std::chrono::duration_cast<TimeDurationMicro>(
      TimeDurationNano(NanoSeconds));
}

/// \brief Converts a duration in nanoseconds (TimeDurationNano) to
/// microseconds.
/// \param NanoSeconds Duration in nanoseconds.
/// \return TimeDurationMicro(double) The corresponding duration in
/// microseconds.
inline TimeDurationMicro nsToMicroseconds(const TimeDurationNano &NanoSeconds) {
  return std::chrono::duration_cast<TimeDurationMicro>(NanoSeconds);
}

/// \brief Converts a duration in nanoseconds (int64_t) to milliseconds.
/// \param NanoSeconds Duration in nanoseconds.
/// \return TimeDurationMilli(double) The corresponding duration in
/// milliseconds.
inline TimeDurationMilli nsToMilliseconds(int64_t NanoSeconds) {
  return std::chrono::duration_cast<TimeDurationMilli>(
      TimeDurationNano(NanoSeconds));
}

/// \brief Converts a duration in nanoseconds (TimeDurationNano) to
/// milliseconds.
/// \param NanoSeconds Duration in nanoseconds.
/// \return TimeDurationMilli(double) The corresponding duration in
/// milliseconds.
inline TimeDurationMilli nsToMilliseconds(const TimeDurationNano &NanoSeconds) {
  return std::chrono::duration_cast<TimeDurationMilli>(NanoSeconds);
}

/// \brief Converts a duration in microseconds (int64_t) to milliseconds.
/// \param MicroSeconds Duration in microseconds.
/// \return TimeDurationMilli(double) The corresponding duration in
/// milliseconds.
inline TimeDurationMilli usToMilliseconds(int64_t MicroSeconds) {
  return std::chrono::duration_cast<TimeDurationMilli>(
      TimeDurationMicro(MicroSeconds));
}

/// \brief Converts a duration in microseconds (TimeDurationMicro) to
/// milliseconds.
/// \param MicroSeconds Duration in microseconds.
/// \return TimeDurationMilli(double) The corresponding duration in
/// milliseconds.
inline TimeDurationMilli
usToMilliseconds(const TimeDurationMicro &MicroSeconds) {
  return std::chrono::duration_cast<TimeDurationMilli>(MicroSeconds);
}

/// \brief Converts a duration in seconds (double) to nanoseconds.
/// \param Seconds Duration in seconds.
/// \return TimeDurationNano(int64_t) The corresponding duration in nanoseconds.
inline TimeDurationNano sToNanoseconds(double Seconds) {
  return std::chrono ::duration_cast<TimeDurationNano>(
      TimeDurationSec(Seconds));
}

/// \brief Converts a duration in seconds (TimeDurationSec) to nanoseconds.
/// \param Seconds Duration in seconds.
/// \return TimeDurationNano(int64_t) The corresponding duration in
/// nanoseconds.
inline TimeDurationNano sToNanoseconds(const TimeDurationSec &Seconds) {
  return std::chrono::duration_cast<TimeDurationNano>(Seconds);
}

/// \brief Converts a duration in seconds (double) to milliseconds.
/// \param Seconds Duration in seconds.
/// \return TimeDurationMilli(double) The corresponding duration in
/// milliseconds.
inline TimeDurationMilli sToMilliseconds(double Seconds) {
  return std::chrono ::duration_cast<TimeDurationMilli>(
      TimeDurationSec(Seconds));
}

/// \brief Converts a duration in seconds (TimeDurationSec) to milliseconds.
/// \param Seconds Duration in seconds.
/// \return TimeDurationMilli(double) The corresponding duration in
/// milliseconds.
inline TimeDurationMilli sToMilliseconds(const TimeDurationSec &Seconds) {
  return std::chrono::duration_cast<TimeDurationMilli>(Seconds);
}

/// \brief Converts a duration in milliseconds to nanoseconds.
/// \param Miliseconds Duration in milliseconds represented as a double.
/// \return TimeDurationNano(int64_t) The corresponding duration in
/// nanoseconds.
inline TimeDurationNano msToNanoseconds(const TimeDurationMilli &Milliseconds) {
  return std::chrono::duration_cast<TimeDurationNano>(Milliseconds);
}

/// \brief Converts a duration in milliseconds to nanoseconds.
/// \param Miliseconds Duration in milliseconds represented as a double.
/// \return TimeDurationNano The corresponding duration in nanoseconds.
inline TimeDurationNano msToNanoseconds(double Milliseconds) {
  return std::chrono::duration_cast<TimeDurationNano>(
      TimeDurationMilli(Milliseconds));
}

///
/// \brief Convert a time stamp in NS resolution to the wall-clock string format
///
///        Y-M-D H:M:S:MS
///
/// \param TimeStamp The time stamp that should be converted
/// \return A string with the walTrue if the ESSReferenceTime objects are not equal, false
/// otherwise.
///
std::string toString(TimeDurationNano TimeStamp);

/// \class ESSTime
///
/// \brief Represents a timestamp in the ESS (European Spallation Source) time
/// format.
///
/// The ESSTime class provides methods for manipulating and converting ESS
/// timestamps.
class ESSTime {
  uint32_t TimeHigh;
  uint32_t TimeLow;

public:
  static constexpr TimeDurationNano SecInNs =
      std::chrono::duration_cast<TimeDurationNano>(std::chrono::seconds(1));
  static constexpr double ESSClockFreqHz{88052500};
  static constexpr double ESSClockTick{SecInNs.count() / ESSClockFreqHz};

  ///
  /// \brief Default constructor.
  ///
  ESSTime() : TimeHigh(0), TimeLow(0) {}

  ///
  /// \brief Constructor that initializes the ESSTime object with the given high
  /// and low timestamp parts
  ///
  /// \param High The high part of the timestamp.
  /// \param Low The low part of the timestamp.
  ///
  ESSTime(const uint32_t High, const uint32_t Low)
      : TimeHigh(High), TimeLow(Low) {}

  ///
  /// \brief Constructor that initializes the ESSTime object with the given
  /// duration in nanoseconds.
  ///
  /// \param timeInNs The duration in nanoseconds.
  ///
  ESSTime(const TimeDurationNano &TimeInNs)
      : TimeHigh(
            std::chrono::duration_cast<std::chrono::seconds>(TimeInNs).count()),
        TimeLow(round((TimeInNs.count() - (TimeHigh * SecInNs.count())) /
                      ESSClockTick)) {}

  ///
  /// \brief Copy constructor.
  /// \param other The ESSTime object to copy from.
  ///
  ESSTime(const ESSTime &other)
      : TimeHigh(other.TimeHigh), TimeLow(other.TimeLow) {}

  ///
  /// \brief Create an ESSTime object set to current system time based on a high
  /// resolution time
  ///
  static inline ESSTime now() {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    return ESSTime(std::chrono::duration_cast<TimeDurationNano>(now));
  }

  /// \brief Adds the given duration in nanoseconds to the ESSTime object.
  ///
  /// \param nanoseconds The duration in nanoseconds to add.
  ///
  inline void operator+=(const TimeDurationNano &NanoSeconds) {
    uint32_t ticksToAdd =
        static_cast<uint32_t>(NanoSeconds.count() / ESSClockTick);
    this->operator+=(ticksToAdd);
  }

  /// \brief Overloads the '+' operator to add a specified duration in
  /// nanoseconds to an ESSTime object.
  ///
  /// \param NanoSeconds The duration in nanoseconds to add.
  /// \return A new ESSTime object that represents the result of the addition.
  inline ESSTime operator+(const TimeDurationNano &NanoSeconds) const {
    ESSTime result = *this;
    uint32_t ticksToAdd =
        static_cast<uint32_t>(NanoSeconds.count() / ESSClockTick);
    return result + ticksToAdd;
  }

  /// \brief Overloads the '-' operator to subtract another ESSTime object from
  /// this one.
  ///
  /// \param other The ESSTime object to subtract.
  /// \return The time difference between the two ESSTime objects as a
  /// TimeDurationNano.
  inline TimeDurationNano operator-(const ESSTime &other) const {
    TimeDurationNano thisDuration = this->toNS();
    TimeDurationNano otherDuration = other.toNS();

    return thisDuration - otherDuration;
  }

  /// \brief Overloads the '+' operator to add a specified number of ticks to an
  /// ESSTime object.
  ///
  /// \param ticks The number of ticks to add.
  /// \return A new ESSTime object that represents the result of the addition.
  ///
  inline ESSTime operator+(uint32_t Ticks) {
    ESSTime result = *this;
    result += Ticks;
    return result;
  }

  ///
  /// \brief Adds the given number of ESS clock ticks to the ESSTime object.
  /// \param ticks The number of ticks to add.
  ///
  inline void operator+=(uint32_t Ticks) {
    uint32_t ticksToAdd = 0;
    if (Ticks >= ESSClockFreqHz) {
      TimeHigh += static_cast<uint32_t>(Ticks / ESSClockFreqHz);
      ticksToAdd += static_cast<uint32_t>(std::fmod(Ticks, ESSClockFreqHz));
    } else {
      ticksToAdd += Ticks;
    }

    if (TimeLow + ticksToAdd >= ESSClockFreqHz) {
      TimeHigh++;
      TimeLow = TimeLow + ticksToAdd - ESSClockFreqHz;
    } else {
      TimeLow += ticksToAdd;
    }
  }

  ///
  /// \brief Assignment operator.
  /// \param other The ESSTime object to assign from.
  /// \return A reference to the assigned ESSTime object.
  ///
  inline ESSTime &operator=(const ESSTime &other) {
    if (this != &other) {
      TimeHigh = other.TimeHigh;
      TimeLow = other.TimeLow;
    }

    return *this;
  }

  ///
  /// \brief Equality comparison operator.
  /// \param other The ESSTime object to compare with.
  /// \return True if both ESSTime objects are equal, false otherwise.
  ///
  inline bool operator==(const ESSTime &other) const {
    return (TimeHigh == other.TimeHigh) && (TimeLow == other.TimeLow);
  }

  ///
  /// \brief Inequality comparison operator.
  /// \param other The ESSTime object to compare with.
  /// \return True if the ESSTime objects are not equal, false otherwise.
  ///
  inline bool operator!=(const ESSTime &other) const {
    return !(*this == other);
  }

  ///
  /// \brief Converts the ESSTime object to a duration in nanoseconds.
  /// \return The duration in nanoseconds.
  ///
  inline TimeDurationNano toNS() const { return toNS(TimeHigh, TimeLow); }

  ///
  /// \brief Static method to convert the given high and low values to a
  /// duration in nanoseconds.
  ///
  /// \param High The high part of the timestamp.
  /// \param Low The low part of the timestamp.
  /// \return The duration in nanoseconds.
  ///
  inline static TimeDurationNano toNS(uint32_t High, uint32_t Low) {
    return TimeDurationNano(High * SecInNs.count() +
                            (uint64_t)(Low * ESSClockTick));
  }

  ///
  /// \brief Returns the high part of the timestamp.
  /// \return The high part of the timestamp.
  ///
  inline uint32_t getTimeHigh() const { return TimeHigh; }

  ///
  /// \brief Returns the low part of the timestamp.
  /// \return The low part of the timestamp.
  ///
  inline uint32_t getTimeLow() const { return TimeLow; }
};

///
/// \class ESSReferenceTime
///
/// \brief Represents a reference time for calculating time-of-flight (TOF)
/// values.
///
/// The ESSReferenceTime class provides methods for setting and retrieving
/// reference times, as well as calculating TOF values based on the reference
/// time and event times.
///
class ESSReferenceTime {
public:
  // clang-format off
  static inline const std::string TOF_COUNT_NAME = "events.timestamp.tof.count";
  static inline const std::string TOF_NEGATIVE_NAME = "events.timestamp.tof.negative";
  static inline const std::string TOF_HIGH_NAME = "events.timestamp.tof.high";
  static inline const std::string PREV_TOF_COUNT_NAME = "events.timestamp.prevtof.count";
  static inline const std::string PREV_TOF_NEGATIVE_NAME = "events.timestamp.prevtof.negative";
  static inline const std::string PREV_TOF_HIGH_NAME = "events.timestamp.prevtof.high";
  // clang-format on
  struct StatCounters : public StatCounterBase {
    int64_t TofCount{0};
    int64_t TofNegative{0};
    int64_t PrevTofCount{0};
    int64_t PrevTofNegative{0};
    int64_t TofHigh{0};
    int64_t PrevTofHigh{0};

    StatCounters(Statistics &Stats)
        : StatCounterBase(Stats, {{TOF_COUNT_NAME, TofCount},
                                  {TOF_NEGATIVE_NAME, TofNegative},
                                  {TOF_HIGH_NAME, TofHigh},
                                  {PREV_TOF_COUNT_NAME, PrevTofCount},
                                  {PREV_TOF_NEGATIVE_NAME, PrevTofNegative},
                                  {PREV_TOF_HIGH_NAME, PrevTofHigh}}) {}
  } Counters;

  /// Counters
  ///  \brief Default constructor.
  ///
  ESSReferenceTime() = delete;

  ///
  /// \brief Constructor that initializes the ESSReferenceTime object with
  /// Statistics.
  ///
  ESSReferenceTime(Statistics &Stats) : Counters(Stats) {}

  /// \brief Constructor that initializes the ESSReferenceTime object with the
  /// given pulse time.
  ///
  /// \param pulseTime The pulse time used as the reference time.
  ///
  ESSReferenceTime(const ESSTime &PulseTime, Statistics &Stats)
      : ESSReferenceTime(Stats) {
    setReference(PulseTime);
  }

  ///
  /// \brief Sets the reference time.
  ///
  /// \param refESSTime The reference time to set in ESSTime format.
  /// \return The reference time as a 64-bit unsigned integer.
  ///
  uint64_t setReference(const ESSTime &RefESSTime);

  ///
  /// \brief Sets the reference time.
  ///
  /// \param refTime The reference time to set in uint64_t format.
  /// \return The reference time as a 64-bit unsigned integer.
  ///
  uint64_t setReference(uint64_t RefTime);

  ///
  /// \brief Sets the previous reference time.
  ///
  /// \param refPrevESSTime The previous reference time to set.
  /// \return The previous reference time as a 64-bit unsigned integer.
  ///
  uint64_t setPrevReference(const ESSTime &RefPrevESSTime);

  ///
  /// \brief Sets the maximum TOF value.
  ///
  /// \param NewMaxTOF The maximum TOF value to set.
  ///
  void setMaxTOF(uint64_t NewMaxTOF);

  ///
  /// \brief Returns the reference time as an ESSTime object.
  /// \return The reference time as an ESSTime object.
  ///
  inline ESSTime getRefESSTime() const { return ESSTime(TimeInNS); }

  ///
  /// \brief Returns the previous reference time as an ESSTime object.
  /// \return The previous reference time as an ESSTime object.
  ///
  inline ESSTime getPrevRefESSTime() const { return ESSTime(PrevTimeInNS); }

  ///
  /// \brief Returns the reference time as a duration in nanoseconds.
  /// \return The reference time as a duration in nanoseconds.
  ///
  inline TimeDurationNano getRefTimeNS() const { return TimeInNS; }

  ///
  /// \brief Returns the previous reference time as a duration in nanoseconds.
  /// \return The previous reference time as a duration in nanoseconds.
  ///
  inline TimeDurationNano getPrevRefTimeNS() const { return PrevTimeInNS; }

  ///
  /// \brief Returns the reference time as a 64-bit unsigned integer.
  /// \return The reference time as a 64-bit unsigned integer.
  ///
  inline uint64_t getRefTimeUInt64() const { return TimeInNS.count(); }

  ///
  /// \brief Returns the previous reference time as a 64-bit unsigned integer.
  /// \return The previous reference time as a 64-bit unsigned integer.
  ///
  inline uint64_t getPrevRefTimeUInt64() const { return PrevTimeInNS.count(); }

  ///
  /// \brief Calculates the time-of-flight (TOF) value based on the saved
  /// reference time and the current event time.
  ///
  /// \param EventEssTime The current event time.
  /// \param DelayNS The delay in nanoseconds.
  /// \return The calculated TOF value.
  ///
  tof_t getTOF(const ESSTime &EventEssTime, uint32_t DelayNS = 0);

  ///
  /// \brief Calculates the previous time-of-flight (TOF) value based on the
  /// saved reference time and the current event time.
  ///
  /// \param EventTime The current event time.
  /// \param DelayNS The delay in nanoseconds.
  /// \return The calculated previous TOF value.
  ///
  tof_t getPrevTOF(const ESSTime &EventTime, uint32_t DelayNS = 0);

  ///
  /// \brief Equality comparison operator.
  ///
  /// \param other The ESSReferenceTime object to compare with.
  /// \return True if both ESSReferenceTime objects are equal, false otherwise.
  ///
  inline bool operator==(const ESSReferenceTime &other) const {
    return (TimeInNS == other.TimeInNS) &&
           (PrevTimeInNS == other.PrevTimeInNS) && (MaxTOF == other.MaxTOF);
  }

  ///
  /// \brief Inequality comparison operator.
  ///
  /// \param other The ESSReferenceTime object to compare with.
  /// \return True if the ESSReferenceTime objects are not equal, false
  /// otherwise.
  ///
  inline bool operator!=(const ESSReferenceTime &other) const {
    return !(*this == other);
  }

private:
  TimeDurationNano TimeInNS{0};
  TimeDurationNano PrevTimeInNS{0};
  /// \todo review that this still appropriate to have 32 bit integer limit
  TimeDurationNano MaxTOF{
      std::numeric_limits<int32_t>::max()}; // max 32 bit integer, larger TOFs
                                            // cause errors downstream
};

} // namespace esstime
