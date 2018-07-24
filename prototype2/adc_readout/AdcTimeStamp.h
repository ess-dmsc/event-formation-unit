/// Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file
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

/// \brief Maps to the timestamps as received from (and used by) the ADC
/// hardware.
struct RawTimeStamp {
  RawTimeStamp() = default;
  RawTimeStamp(std::uint32_t Sec, std::uint32_t SecFrac)
      : Seconds(Sec), SecondsFrac(SecFrac) {}

  /// \brief Number of seconds since EPICS epoch (1980-01-01 00:00).
  /// This value is provided originally by the MRF hardware.
  /// \note The epoch might change in the future.
  std::uint32_t Seconds{0};

  /// \brief Number of clock cycles of the timing hardware clock divided by two.
  // This value resets to zero once every second and has a maximum value of ≈44
  // million.
  // @note It is possible that in future systems, the clock frequency will not
  // be 44 MHz.
  std::uint32_t SecondsFrac{0};

  void fixEndian() {
    Seconds = ntohl(Seconds);
    SecondsFrac = ntohl(SecondsFrac);
  }

  /// @brief Calculate the raw timestamp based on sample offset.
  /// Adds or subtracts the specified number of clock cycles (samples).
  /// Correctly wraps around the fractional seconds if so required to get the
  /// correct value.
  /// @param[in] SampleOffset The number of samples that the time stamp should
  /// be offset. Can be negative and positive.
  /// @return The resulting new timestamp.
  RawTimeStamp GetOffsetTimeStamp(const std::int32_t &SampleOffset) const;

  /// @brief Convert from raw timestamp (seconds plus fractional seconds) into
  /// nanoseconds since epoch.
  /// This version uses floating point math to do the conversion and is likely
  /// to be slower than RawTimeStamp::GetTimeStampNSFast() on machines with a
  /// slow floating point unit.
  /// @return Number of nanoseconds since epoch (currently EPICS epoch).
  std::uint64_t GetTimeStampNS() const;

  /// @brief Convert from raw timestamp (seconds plus fractional seconds) into
  /// nanoseconds since epoch.
  /// This version uses integer math to do the conversion and is likely to as
  /// fast as or slower than RawTimeStamp::GetTimeStampNS() on CPUs that have a
  /// fast floating point unit.
  /// @return Number of nanoseconds since epoch (currently EPICS epoch).
  std::uint64_t GetTimeStampNSFast() const;
} __attribute__((packed));
