// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Holder for readout data (DTO) objects used for
//         timepix
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <cstdint>

#define PIXEL_MAX_TIMESTAMP_NS 26843545600

#define TDC_CLOCK_BIN_NS 3.125
#define TDC_FINE_CLOCK_BIN_NS 0.26

namespace timepixDTO {

struct ESSGlobalTimeStamp {
  const uint64_t pulseTimeInEpochNs;
  const uint64_t tdcClockInPixelTime;

  ESSGlobalTimeStamp(const uint64_t &pulseTimeInEpochNs,
                     const uint64_t &tdcClockInPixelTime)
      : pulseTimeInEpochNs(pulseTimeInEpochNs),
        tdcClockInPixelTime(tdcClockInPixelTime) {}

  bool operator==(const ESSGlobalTimeStamp &other) const {
    return pulseTimeInEpochNs == other.pulseTimeInEpochNs &&
           tdcClockInPixelTime == other.tdcClockInPixelTime;
  }
};

struct EVRDataEvent {
  const uint32_t counter;
  const uint64_t pulseTimeInEpochNs;
  const std::chrono::high_resolution_clock::time_point arrivalTimestamp;

  EVRDataEvent(const uint32_t counter, const uint32_t pulseTimeSeconds,
               const uint32_t pulseTimeNanoSeconds)
      : counter(counter),
        pulseTimeInEpochNs(pulseTimeSeconds * 1e9 + pulseTimeNanoSeconds),
        arrivalTimestamp(std::chrono::high_resolution_clock::now()) {}

  // Equality comparison operator
  bool operator==(const EVRDataEvent &other) const {
    return counter == other.counter &&
           pulseTimeInEpochNs == other.pulseTimeInEpochNs &&
           arrivalTimestamp == other.arrivalTimestamp;
  }
};

struct TDCDataEvent {
  const uint16_t counter;
  const uint64_t tdcTimeStamp;
  const uint8_t pixelClockQuarter;
  const uint32_t tdcTimeInPixelClock;

  const std::chrono::high_resolution_clock::time_point arrivalTimestamp;

  TDCDataEvent(uint16_t triggerCounter, uint64_t timestamp, uint8_t stamp)
      : counter(triggerCounter), tdcTimeStamp(TDC_CLOCK_BIN_NS * timestamp +
                                              TDC_FINE_CLOCK_BIN_NS * stamp),
        pixelClockQuarter(uint8_t(tdcTimeStamp / PIXEL_MAX_TIMESTAMP_NS)),
        tdcTimeInPixelClock(tdcTimeStamp -
                            (PIXEL_MAX_TIMESTAMP_NS * pixelClockQuarter)),
        arrivalTimestamp(std::chrono::high_resolution_clock::now()) {}

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

} // namespace timepixDTO

namespace timepixReadout {

struct EVRReadout {
  const uint8_t type;
  const uint8_t unused;
  const uint16_t unused2;
  const uint32_t counter;
  const uint32_t pulseTimeSeconds;
  const uint32_t pulseTimeNanoSeconds;
  const uint32_t prevPulseTimeSeconds;
  const uint32_t prevPulseTimeNanoSeconds;

  EVRReadout(uint8_t type, uint8_t unused, uint16_t unused2, uint32_t counter,
             uint32_t pulseTimeSeconds, uint32_t pulseTimeNanoSeconds,
             uint32_t prevPulseTimeSeconds, uint32_t prevPulseTimeNanoSeconds)
      : type(type), unused(unused), unused2(unused2), counter(counter),
        pulseTimeSeconds(pulseTimeSeconds),
        pulseTimeNanoSeconds(pulseTimeNanoSeconds),
        prevPulseTimeSeconds(prevPulseTimeSeconds),
        prevPulseTimeNanoSeconds(prevPulseTimeNanoSeconds) {}

  // Equality comparison operator
  bool operator==(const EVRReadout &other) const {
    return type == other.type && unused == other.unused &&
           unused2 == other.unused2 && counter == other.counter &&
           pulseTimeSeconds == other.pulseTimeSeconds &&
           pulseTimeNanoSeconds == other.pulseTimeNanoSeconds &&
           prevPulseTimeSeconds == other.prevPulseTimeSeconds &&
           prevPulseTimeNanoSeconds == other.prevPulseTimeNanoSeconds;
  }
} __attribute__((__packed__));

struct TDCReadout {
  const uint8_t type;
  const uint16_t counter;
  const uint64_t timestamp;
  const uint8_t stamp;

  TDCReadout(uint8_t type, uint16_t counter, uint64_t timestamp, uint8_t stamp)
      : type(type), counter(counter), timestamp(timestamp), stamp(stamp) {}

  bool operator==(const TDCReadout &other) const {
    return type == other.type && counter == other.counter &&
           timestamp == other.timestamp && stamp == other.stamp;
  }
};

struct PixelReadout {
  const uint16_t dCol;
  const uint16_t sPix;
  const uint8_t pix;
  const uint16_t ToT;
  const uint8_t fToA;
  const uint16_t toa;
  const uint32_t spidrTime;

  PixelReadout(uint16_t dCol, uint16_t sPix, uint8_t pix, uint16_t ToT,
               uint8_t fToA, uint16_t toa, uint32_t spidrTime)
      : dCol(dCol), sPix(sPix), pix(pix), ToT(ToT), fToA(fToA), toa(toa),
        spidrTime(spidrTime) {}

  bool operator==(const PixelReadout &other) const {
    return dCol == other.dCol && sPix == other.sPix && pix == other.pix &&
           ToT == other.ToT && fToA == other.fToA && toa == other.toa &&
           spidrTime == other.spidrTime;
  }
};

} // namespace timepixReadout