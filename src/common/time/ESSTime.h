// Copyright (C) 2019-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ESS time related methods and classes
///
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <common/debug/Trace.h>
#include <cstdint>

namespace esstime {

using TimePointNano = std::chrono::high_resolution_clock::time_point;
using TimeDurationNano = std::chrono::duration<size_t, std::nano>;

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

  /// \brief Default constructor.
  ESSTime() : TimeHigh(0), TimeLow(0) {}

  /// \brief Constructor that initializes the ESSTime object with the given high
  /// and low timestamp parts
  ///
  /// \param High The high part of the timestamp.
  /// \param Low The low part of the timestamp.
  ESSTime(const uint32_t High, const uint32_t Low)
      : TimeHigh(High), TimeLow(Low) {}

  /// \brief Constructor that initializes the ESSTime object with the given
  /// duration in nanoseconds.
  ///
  /// \param timeInNs The duration in nanoseconds.
  ESSTime(const TimeDurationNano timeInNs)
      : TimeHigh(
            std::chrono::duration_cast<std::chrono::seconds>(timeInNs).count()),
        TimeLow(round((timeInNs.count() - (TimeHigh * SecInNs.count())) /
                      ESSClockTick)) {}

  /// \brief Copy constructor.
  ///
  /// \param other The ESSTime object to copy from.
  ESSTime(const ESSTime &other)
      : TimeHigh(other.TimeHigh), TimeLow(other.TimeLow) {}

  /// \brief Adds the given duration in nanoseconds to the ESSTime object.
  ///
  /// \param nanoseconds The duration in nanoseconds to add.
  inline void operator+=(const TimeDurationNano &nanoseconds) {
    uint32_t ticksToAdd =
        static_cast<uint32_t>(nanoseconds.count() / ESSClockTick);
    this->operator+=(ticksToAdd);
  }

  /// \brief Overloads the '+' operator to add a specified number of ticks to an
  /// ESSTime object.
  ///
  /// \param ticks The number of ticks to add.
  /// \return A new ESSTime object that represents the result of the addition.
  inline ESSTime operator+(const uint32_t &ticks) {
    ESSTime result = *this;
    result += ticks;
    return result;
  }

  /// \brief Adds the given number of ESS clock ticks to the ESSTime object.
  ///
  /// \param ticks The number of ticks to add.
  inline void operator+=(const uint32_t &ticks) {
    uint32_t ticksToAdd = 0;
    if (ticks >= ESSClockFreqHz) {
      TimeHigh += static_cast<uint32_t>(ticks / ESSClockFreqHz);
      ticksToAdd += static_cast<uint32_t>(std::fmod(ticks, ESSClockFreqHz));
    } else {
      ticksToAdd += ticks;
    }

    if (TimeLow + ticksToAdd >= ESSClockFreqHz) {
      TimeHigh++;
      TimeLow = TimeLow + ticksToAdd - ESSClockFreqHz;
    } else {
      TimeLow += ticksToAdd;
    }
  }

  /// \brief Assignment operator.
  ///
  /// \param other The ESSTime object to assign from.
  /// \return A reference to the assigned ESSTime object.
  ESSTime &operator=(const ESSTime &other) {
    if (this != &other) {
      TimeHigh = other.TimeHigh;
      TimeLow = other.TimeLow;
    }

    return *this;
  }

  /// \brief Converts the ESSTime object to a duration in nanoseconds.
  ///
  /// \return The duration in nanoseconds.
  inline TimeDurationNano toNS() const { return toNS(TimeHigh, TimeLow); }

  /// \brief Static method to convert the given high and low values to a
  /// duration in nanoseconds.
  ///
  /// \param High The high part of the timestamp.
  /// \param Low The low part of the timestamp.
  /// \return The duration in nanoseconds.
  static TimeDurationNano toNS(const uint32_t &High, const uint32_t &Low) {
    return TimeDurationNano(High * SecInNs.count() +
                            (uint64_t)(Low * ESSClockTick));
  }

  /// \brief Returns the high part of the timestamp.
  ///
  /// \return The high part of the timestamp.
  inline uint32_t getTimeHigh() const { return TimeHigh; }

  /// \brief Returns the low part of the timestamp.
  ///
  /// \return The low part of the timestamp.
  inline uint32_t getTimeLow() const { return TimeLow; }
};

/// \class ESSReferenceTime
///
/// \brief Represents a reference time for calculating time-of-flight (TOF)
/// values.
///
/// The ESSReferenceTime class provides methods for setting and retrieving
/// reference times, as well as calculating TOF values based on the reference
/// time and event times.
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

  /// \brief Default constructor.
  ESSReferenceTime() = default;

  /// \brief Constructor that initializes the ESSReferenceTime object with the
  /// given pulse time.
  ///
  /// \param pulseTime The pulse time used as the reference time.
  ESSReferenceTime(ESSTime pulseTime) : TimeInNS(pulseTime.toNS()){};

  const uint64_t InvalidTOF{0xFFFFFFFFFFFFFFFFULL};

  /// \brief Sets the reference time.
  ///
  /// \param refESSTime The reference time to set.
  /// \return The reference time as a 64-bit unsigned integer.
  uint64_t setReference(const ESSTime &refESSTime);

  /// \brief Sets the previous reference time.
  ///
  /// \param refPrevESSTime The previous reference time to set.
  /// \return The previous reference time as a 64-bit unsigned integer.
  uint64_t setPrevReference(const ESSTime &refPrevESSTime);

  /// \brief Sets the maximum TOF value.
  ///
  /// \param NewMaxTOF The maximum TOF value to set.
  void setMaxTOF(const uint64_t NewMaxTOF);

  /// \brief Returns the reference time as an ESSTime object.
  ///
  /// \return The reference time as an ESSTime object.
  inline ESSTime getRefESSTime() const { return ESSTime(TimeInNS); }

  /// \brief Returns the previous reference time as an ESSTime object.
  ///
  /// \return The previous reference time as an ESSTime object.
  inline ESSTime getPrevRefESSTime() const { return ESSTime(PrevTimeInNS); }

  /// \brief Returns the reference time as a duration in nanoseconds.
  ///
  /// \return The reference time as a duration in nanoseconds.
  inline TimeDurationNano getRefTimeNS() const { return TimeInNS; }

  /// \brief Returns the previous reference time as a duration in nanoseconds.
  ///
  /// \return The previous reference time as a duration in nanoseconds.
  inline TimeDurationNano getPrevRefTimeNS() const { return PrevTimeInNS; }

  /// \brief Returns the reference time as a 64-bit unsigned integer.
  ///
  /// \return The reference time as a 64-bit unsigned integer.
  inline uint64_t getRefTimeUInt64() const { return TimeInNS.count(); }

  /// \brief Returns the previous reference time as a 64-bit unsigned integer.
  ///
  /// \return The previous reference time as a 64-bit unsigned integer.
  inline uint64_t getPrevRefTimeUInt64() const { return PrevTimeInNS.count(); }

  /// \brief Calculates the time-of-flight (TOF) value based on the saved
  /// reference time and the current event time.
  ///
  /// \param eventEssTime The current event time.
  /// \param DelayNS The delay in nanoseconds.
  /// \return The calculated TOF value.
  uint64_t getTOF(const ESSTime eventEssTime, const uint32_t DelayNS = 0);

  /// \brief Calculates the previous time-of-flight (TOF) value based on the
  /// saved reference time and the current event time.
  ///
  /// \param eventEssTime The current event time.
  /// \param DelayNS The delay in nanoseconds.
  /// \return The calculated previous TOF value.
  uint64_t getPrevTOF(const ESSTime eventTime, const uint32_t DelayNS = 0);

  struct Stats_t Stats = {};

private:
  TimeDurationNano TimeInNS{0};
  TimeDurationNano PrevTimeInNS{0};
  TimeDurationNano MaxTOF{
      2147483647}; // max 32 bit integer, larger TOFs cause errors downstream
};

} // namespace esstime
