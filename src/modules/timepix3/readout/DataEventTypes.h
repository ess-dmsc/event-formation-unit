// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief container file to define main data transfer objects DTO's used for
//         timepix
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <cstdint>

namespace Timepix3 {

#define TDC_CLOCK_BIN_NS 3.125
#define TDC_FINE_CLOCK_BIN_NS 0.26
#define TDC_MAX_TIMESTAMP_NS 107.3741824 * 1e9
#define PIXEL_MAX_TIMESTAMP_NS 26.8435456 * 1e9

using namespace std::chrono;

struct EVRDataEvent;
struct TDCDataEvent;
struct GlobalTime;

struct TDCDataEvent {
  const uint16_t counter;
  const uint64_t tdcTimeStamp;
  const high_resolution_clock::time_point arrivalTimestamp;

  /// \todo reate copy constructor
  TDCDataEvent(uint16_t triggerCounter, uint64_t timestamp,
               uint8_t stamp)
      : counter(triggerCounter),
        tdcTimeStamp(TDC_CLOCK_BIN_NS * timestamp +
                     TDC_FINE_CLOCK_BIN_NS * stamp),
        arrivalTimestamp(high_resolution_clock::now()) {}

  TDCDataEvent(uint16_t triggerCounter, uint64_t timestamp,
               uint8_t stamp,
               time_point<system_clock> arrivalTimestamp)
      : counter(triggerCounter),
        tdcTimeStamp(TDC_CLOCK_BIN_NS * timestamp +
                     TDC_FINE_CLOCK_BIN_NS * stamp),
        arrivalTimestamp(arrivalTimestamp) {}

  // Equality comparison operator
  bool operator==(const TDCDataEvent &other) const {
    return counter == other.counter &&
           tdcTimeStamp == other.tdcTimeStamp &&
           arrivalTimestamp == other.arrivalTimestamp;
  }
};


struct EVRDataEvent {
  const uint32_t Counter;
  const uint32_t PulseTimeSeconds;
  const uint32_t PulseTimeNanoSeconds;
  const high_resolution_clock::time_point arrivalTimestamp;

  EVRDataEvent(uint32_t counter,
               uint32_t pulseTimeSeconds, uint32_t pulseTimeNanoSeconds)
      : Counter(counter),
        PulseTimeSeconds(pulseTimeSeconds),
        PulseTimeNanoSeconds(pulseTimeNanoSeconds),
        arrivalTimestamp(high_resolution_clock::now()) {}

  EVRDataEvent(uint32_t counter,
               uint32_t pulseTimeSeconds, uint32_t pulseTimeNanoSeconds,
               time_point<system_clock> arrivalTimestamp)
      : Counter(counter),
        PulseTimeSeconds(pulseTimeSeconds),
        PulseTimeNanoSeconds(pulseTimeNanoSeconds),
        arrivalTimestamp(arrivalTimestamp) {}

  // Equality comparison operator
  bool operator==(const EVRDataEvent &other) const {
    return Counter == other.Counter &&
           PulseTimeSeconds == other.PulseTimeSeconds &&
           PulseTimeNanoSeconds == other.PulseTimeNanoSeconds &&
           arrivalTimestamp == other.arrivalTimestamp;
  }
};

struct GlobalTime {
  const uint64_t globalTimeStamp;
  const TDCDataEvent tdcTimeStamp;

  GlobalTime(uint64_t pulseTimeSeconds, uint64_t pulseTimeNanoSeconds, TDCDataEvent tdcTimeStamp):
  globalTimeStamp(pulseTimeSeconds * 1e9 + pulseTimeNanoSeconds), tdcTimeStamp(tdcTimeStamp) {}
};

} // namespace Timepix3