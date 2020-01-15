/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief For doing time stamp calculations.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <netinet/in.h>

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
