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

namespace Timepix3 {

#define TDC_CLOCK_BIN_NS 3.125
#define TDC_FINE_CLOCK_BIN_NS 0.26
#define TDC_MAX_TIMESTAMP_NS 107.3741824 * 1e9
#define PIXEL_MAX_TIMESTAMP_NS 26.8435456 * 1e9

using namespace std::chrono;

struct EVRDataEvent;
struct TDCDataEvent;

struct TDCDataEvent {
  const uint8_t type;
  const uint16_t counter;
  const uint64_t tdcTimeStamp;
  const uint64_t nextTdcStimeStamp;
  const high_resolution_clock::time_point arrivalTimestamp;

  /// \todo reate copy constructor
  TDCDataEvent(uint8_t type, uint16_t triggerCounter, uint64_t timestamp,
               uint8_t stamp, uint32_t frequency)
      : type(type), counter(triggerCounter),
        tdcTimeStamp(TDC_CLOCK_BIN_NS * timestamp +
                     TDC_FINE_CLOCK_BIN_NS * stamp),
        nextTdcStimeStamp(tdcTimeStamp + frequency),
        arrivalTimestamp(high_resolution_clock::now()) {}

  TDCDataEvent(uint8_t type, uint16_t triggerCounter, uint64_t timestamp,
               uint8_t stamp, uint32_t frequency,
               time_point<system_clock> arrivalTimestamp)
      : type(type), counter(triggerCounter),
        tdcTimeStamp(TDC_CLOCK_BIN_NS * timestamp +
                     TDC_FINE_CLOCK_BIN_NS * stamp),
        nextTdcStimeStamp(tdcTimeStamp + frequency),
        arrivalTimestamp(arrivalTimestamp) {}

  // Equality comparison operator
  bool operator==(const TDCDataEvent &other) const {
    return type == other.type && counter == other.counter &&
           tdcTimeStamp == other.tdcTimeStamp &&
           nextTdcStimeStamp == other.nextTdcStimeStamp &&
           arrivalTimestamp == other.arrivalTimestamp;
  }
};

struct EVRDataEvent {
  const uint8_t Type;
  const uint8_t Unused;
  const uint16_t Unused2;
  const uint32_t Counter;
  const uint32_t PulseTimeSeconds;
  const uint32_t PulseTimeNanoSeconds;
  const uint32_t PrevPulseTimeSeconds;
  const uint32_t PrevPulseTimeNanoSeconds;
  const high_resolution_clock::time_point arrivalTimestamp;

  EVRDataEvent(uint8_t type, uint8_t unused, uint16_t unused2, uint32_t counter,
               uint32_t pulseTimeSeconds, uint32_t pulseTimeNanoSeconds,
               uint32_t prevPulseTimeSeconds, uint32_t prevPulseTimeNanoSeconds)
      : Type(type), Unused(unused), Unused2(unused2), Counter(counter),
        PulseTimeSeconds(pulseTimeSeconds),
        PulseTimeNanoSeconds(pulseTimeNanoSeconds),
        PrevPulseTimeSeconds(prevPulseTimeSeconds),
        PrevPulseTimeNanoSeconds(prevPulseTimeNanoSeconds),
        arrivalTimestamp(high_resolution_clock::now()) {}

  EVRDataEvent(uint8_t type, uint8_t unused, uint16_t unused2, uint32_t counter,
               uint32_t pulseTimeSeconds, uint32_t pulseTimeNanoSeconds,
               uint32_t prevPulseTimeSeconds, uint32_t prevPulseTimeNanoSeconds,
               time_point<system_clock> arrivalTimestamp)
      : Type(type), Unused(unused), Unused2(unused2), Counter(counter),
        PulseTimeSeconds(pulseTimeSeconds),
        PulseTimeNanoSeconds(pulseTimeNanoSeconds),
        PrevPulseTimeSeconds(prevPulseTimeSeconds),
        PrevPulseTimeNanoSeconds(prevPulseTimeNanoSeconds),
        arrivalTimestamp(arrivalTimestamp) {}

  // Equality comparison operator
  bool operator==(const EVRDataEvent &other) const {
    return Type == other.Type && Unused == other.Unused &&
           Unused2 == other.Unused2 && Counter == other.Counter &&
           PulseTimeSeconds == other.PulseTimeSeconds &&
           PulseTimeNanoSeconds == other.PulseTimeNanoSeconds &&
           PrevPulseTimeSeconds == other.PrevPulseTimeSeconds &&
           PrevPulseTimeNanoSeconds == other.PrevPulseTimeNanoSeconds &&
           arrivalTimestamp == other.arrivalTimestamp;
  }
};
} // namespace Timepix3