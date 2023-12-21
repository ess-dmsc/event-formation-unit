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
struct EpochESSPulseTime;

struct TDCDataEvent {
  const uint16_t counter;
  const uint64_t tdcTimeStamp;
  const uint8_t pixelClockQuarter;
  const uint32_t tdcTimeInPixelClock;

  const high_resolution_clock::time_point arrivalTimestamp;

  /// \todo reate copy constructor
  TDCDataEvent(uint16_t triggerCounter, uint64_t timestamp, uint8_t stamp)
      : counter(triggerCounter), tdcTimeStamp(TDC_CLOCK_BIN_NS * timestamp +
                                              TDC_FINE_CLOCK_BIN_NS * stamp),
        pixelClockQuarter(uint8_t(tdcTimeStamp / PIXEL_MAX_TIMESTAMP_NS)),
        tdcTimeInPixelClock(tdcTimeStamp -
                            (PIXEL_MAX_TIMESTAMP_NS * pixelClockQuarter)),
        arrivalTimestamp(high_resolution_clock::now()) {}

  TDCDataEvent(uint16_t triggerCounter, uint64_t timestamp, uint8_t stamp,
               time_point<system_clock> arrivalTimestamp)
      : counter(triggerCounter), tdcTimeStamp(TDC_CLOCK_BIN_NS * timestamp +
                                              TDC_FINE_CLOCK_BIN_NS * stamp),

        pixelClockQuarter(uint8_t(tdcTimeStamp / PIXEL_MAX_TIMESTAMP_NS)),
        tdcTimeInPixelClock(tdcTimeStamp -
                            (PIXEL_MAX_TIMESTAMP_NS * pixelClockQuarter)),
        arrivalTimestamp(arrivalTimestamp) {}

  // Equality comparison operator
  bool operator==(const TDCDataEvent &other) const {
    return counter == other.counter && tdcTimeStamp == other.tdcTimeStamp &&
           pixelClockQuarter == other.pixelClockQuarter &&
           tdcTimeInPixelClock == other.tdcTimeInPixelClock &&
           arrivalTimestamp == other.arrivalTimestamp;
  }

private:
  inline uint64_t
  convertTdcTimeToPixelTime(const uint64_t tdcTimeToConvert) const {
    return tdcTimeToConvert -
           (PIXEL_MAX_TIMESTAMP_NS *
            uint16_t(tdcTimeToConvert / PIXEL_MAX_TIMESTAMP_NS));
  }
};

struct EVRDataEvent {
  const uint32_t counter;
  const uint32_t pulseTimeSeconds;
  const uint32_t pulseTimeNanoSeconds;
  const high_resolution_clock::time_point arrivalTimestamp;

  EVRDataEvent(uint32_t counter, uint32_t pulseTimeSeconds,
               uint32_t pulseTimeNanoSeconds)
      : counter(counter), pulseTimeSeconds(pulseTimeSeconds),
        pulseTimeNanoSeconds(pulseTimeNanoSeconds),
        arrivalTimestamp(high_resolution_clock::now()) {}

  EVRDataEvent(uint32_t counter, uint32_t pulseTimeSeconds,
               uint32_t pulseTimeNanoSeconds,
               time_point<system_clock> arrivalTimestamp)
      : counter(counter), pulseTimeSeconds(pulseTimeSeconds),
        pulseTimeNanoSeconds(pulseTimeNanoSeconds),
        arrivalTimestamp(arrivalTimestamp) {}

  // Equality comparison operator
  bool operator==(const EVRDataEvent &other) const {
    return counter == other.counter &&
           pulseTimeSeconds == other.pulseTimeSeconds &&
           pulseTimeNanoSeconds == other.pulseTimeNanoSeconds &&
           arrivalTimestamp == other.arrivalTimestamp;
  }
};

struct EpochESSPulseTime {
  const uint64_t pulseTimeInEpochNs;
  const TDCDataEvent pairedTDCDataEvent;

  EpochESSPulseTime(uint64_t pulseTimeSeconds, uint64_t pulseTimeNanoSeconds,
                    TDCDataEvent &tdcTimeStamp)
      : pulseTimeInEpochNs(pulseTimeSeconds * 1e9 + pulseTimeNanoSeconds),
        pairedTDCDataEvent(tdcTimeStamp) {}

  bool operator==(const EpochESSPulseTime &other) const {
    return pulseTimeInEpochNs == other.pulseTimeInEpochNs &&
           pairedTDCDataEvent == other.pairedTDCDataEvent;
  }
};

} // namespace Timepix3